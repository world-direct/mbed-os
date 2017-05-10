/*
 * ubirch#1 M66 Modem core functionality interface.
 *
 * @author Niranjan Rao
 * @date 2017-02-09
 *
 * @copyright &copy; 2015, 2016 ubirch GmbH (https://ubirch.com)
 *
 * ```
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ```
 */

#include <string.h>
#include "mbed_debug.h"

#include "QuectelM66Interface.h"
#include "SerialStreamAdapter.h"
#include "SerialBuffer/BufferedSerial.h"

extern "C" {
	#include "lwip/lwip_quectelm66_stack.h"
}


// Various timeouts for different ESP8266 operations
#define M66_CONNECT_TIMEOUT 15000
#define M66_SEND_TIMEOUT    500
#define M66_RECV_TIMEOUT    0
#define M66_MISC_TIMEOUT    500


QuectelM66Interface::QuectelM66Interface(PinName tx, PinName rx, PinName pwrKey, PinName vdd_ext, const char *apn, const char *userName, const char *passPhrase)
	: QuectelM66Interface(new SerialStreamAdapter(new BufferedSerial(tx, rx)), pwrKey, vdd_ext, apn, userName, passPhrase) {
		
	}

QuectelM66Interface::QuectelM66Interface(SerialStreamAdapter* serialStreamAdapter, PinName pwrKey, PinName vdd_ext, const char *apn, const char *userName, const char *passPhrase)
	: _commandCoordinator(serialStreamAdapter, pwrKey, vdd_ext, apn, userName, passPhrase)
	, _dhcp(true)
	, _ip_address()
	, _netmask()
	, _gateway()
	, _processingThread(osPriorityNormal, 8192) {
		
	debug("QuectelM66Interface --> ctor");
	this->_serialStreamAdapter = serialStreamAdapter;
		
	if (!this->_commandCoordinator.startup()) {
		debug("QuectelM66Interface --> Interface could not be started, system reset");
		NVIC_SystemReset();
	}
		
}

QuectelM66Interface::~QuectelM66Interface() {
	
	debug("QuectelM66Interface --> dtor");
	
	if (!this->_commandCoordinator.shutdown()) {
		debug("QuectelM66Interface --> Interface couldn't be brought down, system reset");
		NVIC_SystemReset();
	}
	
}

const char *QuectelM66Interface::get_mac_address() {
	debug("QuectelM66Interface --> get_mac_address");
	return NULL;
}

const char *QuectelM66Interface::get_ip_address() {
	debug("QuectelM66Interface --> get_ip_address");
	if (mbed_lwip_quectelm66_get_ip_address(_ip_address, sizeof _ip_address)) {
		return _ip_address;
	}
	return NULL;
}

const char *QuectelM66Interface::get_netmask() {
	debug("QuectelM66Interface --> get_netmask");
	if (mbed_lwip_quectelm66_get_netmask(_netmask, sizeof _netmask)) {
		return _netmask;
	}
	return 0;
}

const char *QuectelM66Interface::get_gateway() {
	debug("QuectelM66Interface --> get_gateway");
	if (mbed_lwip_quectelm66_get_gateway(_gateway, sizeof _gateway)) {
		return _gateway;
	}
	return 0;
}


nsapi_error_t QuectelM66Interface::set_network(const char *ip_address, const char *netmask, const char *gateway) {
	debug("QuectelM66Interface --> set_network");
	_dhcp = false;
	strncpy(_ip_address, ip_address ? ip_address : "", sizeof(_ip_address));
	strncpy(_netmask, netmask ? netmask : "", sizeof(_netmask));
	strncpy(_gateway, gateway ? gateway : "", sizeof(_gateway));
	return NSAPI_ERROR_OK;
}


nsapi_error_t QuectelM66Interface::set_dhcp(bool dhcp) {
	debug("QuectelM66Interface --> set_dhcp");
	_dhcp = dhcp;
	return NSAPI_ERROR_OK;
}

static SerialStreamAdapter* serialStreamAdapterWrapper;

static int mbed_set_serial_io_fns_wrapper_read(uint8_t* buf, size_t* pLength, size_t maxLength, uint32_t timeout /* = osWaitForever */) {
	return serialStreamAdapterWrapper->read(buf, pLength, maxLength, timeout);
}

static int mbed_set_serial_io_fns_wrapper_write(uint8_t* buf, size_t length, uint32_t timeout /* = osWaitForever */) {
	return serialStreamAdapterWrapper->write(buf, length, timeout);
}

nsapi_error_t QuectelM66Interface::connect() {
	debug("QuectelM66Interface --> connect");
	
	serialStreamAdapterWrapper = this->_serialStreamAdapter;
	
	_processingThread.start(mbed::Callback<void()>(this, &QuectelM66Interface::serial_read_callback));
	
	serial_io_fns fns;
	fns.read = mbed_set_serial_io_fns_wrapper_read;
	fns.write = mbed_set_serial_io_fns_wrapper_write;
	mbed_set_serial_io_fns(fns);
	
	if (mbed_lwip_quectelm66_bringup(
			_dhcp,
            _ip_address[0] ? _ip_address : 0,
            _netmask[0] ? _netmask : 0,
            _gateway[0] ? _gateway : 0
		) != NSAPI_ERROR_OK) {
			return NSAPI_ERROR_NO_CONNECTION;
	}
	
	return NSAPI_ERROR_OK;
	
}

void QuectelM66Interface::serial_read_callback() {
	
	while (true) {
	
		u8_t buffer[1024];
		int size;
	
		if (this->_serialStreamAdapter->read(buffer, &size, 1024, 50) == 0) {
			ppp_pcb *tmp = mbed_get_ppp_pcb();
			pppos_input_tcpip(tmp, buffer, size);	
		}
		
	}
	
}

	    
nsapi_error_t QuectelM66Interface::disconnect() {
	debug("QuectelM66Interface --> disconnect");
	return mbed_lwip_quectelm66_bringdown();
}

NetworkStack *QuectelM66Interface::get_stack() {
	debug("QuectelM66Interface --> get_stack");
	return nsapi_create_stack(&lwip_quectelm66_stack);
}
