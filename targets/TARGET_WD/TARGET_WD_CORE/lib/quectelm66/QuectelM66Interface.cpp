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
#include "wd_logging.h"

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
	, _readProcessingThread(osPriorityNormal)
	, _readNotificationQueue(){
		
	wd_log_debug("QuectelM66Interface --> ctor");
		
	this->_serialStreamAdapter = serialStreamAdapter;	
		
	this->_readProcessingThread.start(mbed::Callback<void()>(this, &QuectelM66Interface::serial_read_thread_entry));
		
	if (!this->_commandCoordinator.startup()) {
		wd_log_error("QuectelM66Interface --> Interface could not be started, system reset");
		mbed_die();
	}
		
}

QuectelM66Interface::~QuectelM66Interface() {
	
	wd_log_debug("QuectelM66Interface --> dtor");
	
	if (!this->_commandCoordinator.shutdown()) {
		wd_log_debug("QuectelM66Interface --> Interface couldn't be brought down, system reset");
		mbed_die();
	}
	
}

const char *QuectelM66Interface::get_mac_address() {
	wd_log_debug("QuectelM66Interface --> get_mac_address");
	return NULL;
}

const char *QuectelM66Interface::get_ip_address() {
	wd_log_debug("QuectelM66Interface --> get_ip_address");
	if (mbed_lwip_quectelm66_get_ip_address(_ip_address, sizeof _ip_address)) {
		return _ip_address;
	}
	return NULL;
}

const char *QuectelM66Interface::get_netmask() {
	wd_log_debug("QuectelM66Interface --> get_netmask");
	if (mbed_lwip_quectelm66_get_netmask(_netmask, sizeof _netmask)) {
		return _netmask;
	}
	return 0;
}

const char *QuectelM66Interface::get_gateway() {
	wd_log_debug("QuectelM66Interface --> get_gateway");
	if (mbed_lwip_quectelm66_get_gateway(_gateway, sizeof _gateway)) {
		return _gateway;
	}
	return 0;
}


nsapi_error_t QuectelM66Interface::set_network(const char *ip_address, const char *netmask, const char *gateway) {
	wd_log_debug("QuectelM66Interface --> set_network");
	_dhcp = false;
	strncpy(_ip_address, ip_address ? ip_address : "", sizeof(_ip_address));
	strncpy(_netmask, netmask ? netmask : "", sizeof(_netmask));
	strncpy(_gateway, gateway ? gateway : "", sizeof(_gateway));
	return NSAPI_ERROR_OK;
}


nsapi_error_t QuectelM66Interface::set_dhcp(bool dhcp) {
	wd_log_debug("QuectelM66Interface --> set_dhcp");
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
	wd_log_info("QuectelM66Interface --> connect");
	
	serialStreamAdapterWrapper = this->_serialStreamAdapter;
	
	//this->_serialStreamAdapter->attach(mbed::Callback<void()>(this, &QuectelM66Interface::serial_read_notify), SerialBase::RxIrq);
	
	this->_readProcessingThread.signal_set(QUECTEL_M66_PPP_READ_START_SIGNAL);
	
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

//void QuectelM66Interface::serial_read_notify() {
//	
//	//wd_log_debug("QuectelM66Interface --> Performing serial-read notification for ppp");
//	//_readNotificationQueue.put(this);
//	this->_readProcessingThread.signal_set(QUECTEL_M66_PPP_READ_DATA_SIGNAL);
//	
//}

void QuectelM66Interface::serial_read_thread_entry() {
	
	wd_log_debug("QuectelM66Interface --> serial_read_thread_entry");
	
	this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_START_SIGNAL);
	this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_STOP_SIGNAL);
	//this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_DATA_SIGNAL);
	
	do {
	
		this->_readProcessingThread.signal_wait(QUECTEL_M66_PPP_READ_START_SIGNAL);
		wd_log_debug("QuectelM66Interface --> QUECTEL_M66_PPP_READ_START_SIGNAL received");
	
		int size;
		do {
		
			if (this->_serialStreamAdapter->read(this->_serialBuffer, &size, QUECTEL_M66_READ_BUFFER_SIZE, 1000) == 0) {
				wd_log_debug("QuectelM66Interface --> %d bytes read, forwarding to pppos", size);
				ppp_pcb *tmp = mbed_get_ppp_pcb();
				pppos_input_tcpip(tmp, this->_serialBuffer, size);	
			}
			
		} while (this->_readProcessingThread.signal_wait(QUECTEL_M66_PPP_READ_STOP_SIGNAL, 0).status != osEventSignal);
		wd_log_debug("QuectelM66Interface --> QUECTEL_M66_PPP_READ_STOP_SIGNAL received");
	
	} while (true);
		
}

	    
nsapi_error_t QuectelM66Interface::disconnect() {
	wd_log_info("QuectelM66Interface --> disconnect");
	nsapi_error_t ret = mbed_lwip_quectelm66_bringdown();
	this->_readProcessingThread.signal_set(QUECTEL_M66_PPP_READ_STOP_SIGNAL);
	return ret;
}

NetworkStack *QuectelM66Interface::get_stack() {
	wd_log_debug("QuectelM66Interface --> get_stack");
	return nsapi_create_stack(&lwip_quectelm66_stack);
}
