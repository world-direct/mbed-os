/*
 * QuectelM66Interface.cpp
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created: 21.04.2017
 * Author:	Simon Pfeifhofer
 * EMail:	simon.pfeifhofer@world-direct.at
 *
 * Description:
 *	 Contains the NetworkInterface implementation for the modem.
 *   Uses the QuectelM66CommandCoordinator for the power-on sequence and PPP-dialing.
 *   Uses LWIP PPPOS for the PPP-communication.
 *
 */ 

#include <string.h>
#include "wd_logging.h"

#include "QuectelM66Interface.h"
#include "SerialStreamAdapter.h"
#include "BufferedSerial.h"

extern "C" {
	#include "lwip_quectelm66_stack.h"
}


// Various timeouts for different ESP8266 operations
#define M66_CONNECT_TIMEOUT 15000
#define M66_SEND_TIMEOUT    500
#define M66_RECV_TIMEOUT    0
#define M66_MISC_TIMEOUT    500


QuectelM66Interface::QuectelM66Interface(PinName tx, PinName rx, PinName pwrKey, PinName vdd_ext, const char *apn, const char *username, const char *password)
	: QuectelM66Interface(new SerialStreamAdapter(new BufferedSerial(tx, rx)), pwrKey, vdd_ext, apn, username, password) {
	}

QuectelM66Interface::QuectelM66Interface(SerialStreamAdapter* serialStreamAdapter, PinName pwrKey, PinName vdd_ext, const char *apn, const char *username, const char *password)
	: _commandCoordinator(serialStreamAdapter, pwrKey, vdd_ext, apn)
	, _dhcp(true)
	, _ip_address()
	, _netmask()
	, _gateway()
	, _apn(apn)
	, _username(username)
	, _password(password)
	, _readProcessingThread(osPriorityNormal)
	, _readNotificationQueue(){
		
	wd_log_debug("QuectelM66Interface --> ctor");
		
	this->_serialStreamAdapter = serialStreamAdapter;	
	
	this->_pppos_ctx.serial_stream_adapter = serialStreamAdapter;
	this->_pppos_ctx.pppos_write_callback = &QuectelM66Interface::pppos_write_wrapper;
		
	this->_readProcessingThread.start(mbed::Callback<void()>(this, &QuectelM66Interface::serial_read_thread_entry));
		
	if (!this->_commandCoordinator.startup()) {
		wd_log_error("QuectelM66Interface --> Interface could not be started, system reset");
		NVIC_SystemReset();
	}
		
	wd_log_info("QuectelM66Interface --> RSSI: %d", this->_commandCoordinator.GetRSSI());
	wd_log_info("QuectelM66Interface --> GSMRegistrationState: %d", this->_commandCoordinator.GetGSMRegistrationState());
	wd_log_info("QuectelM66Interface --> GPRSRegistrationState: %d", this->_commandCoordinator.GetGPRSRegistrationState());
	wd_log_info("QuectelM66Interface --> Bearer: %d", this->_commandCoordinator.GetBearer());
	wd_log_info("QuectelM66Interface --> Phone-number: %s", this->_commandCoordinator.GetPhoneNumber());
		
}

QuectelM66Interface::~QuectelM66Interface() {
	
	wd_log_debug("QuectelM66Interface --> dtor");
	
	if (!this->_commandCoordinator.shutdown()) {
		wd_log_debug("QuectelM66Interface --> Interface couldn't be brought down, system reset");
		NVIC_SystemReset();
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

int QuectelM66Interface::pppos_write_wrapper(pppos_context_t* pppos_context, uint8_t* buf, size_t length, uint32_t timeout /* = osWaitForever */) {
	return ((SerialStreamAdapter*)(pppos_context->serial_stream_adapter))->write(buf, length, timeout);
}

nsapi_error_t QuectelM66Interface::connect() {
	wd_log_info("QuectelM66Interface --> connect");
	
	this->_readProcessingThread.signal_set(QUECTEL_M66_PPP_READ_START_SIGNAL);
	
	if (mbed_lwip_quectelm66_bringup(
            _ip_address[0] ? _ip_address : 0,
            _netmask[0] ? _netmask : 0,
            _gateway[0] ? _gateway : 0,
			this->_username,
			this->_password,
			&(this->_pppos_ctx)
		) != NSAPI_ERROR_OK) {
			return NSAPI_ERROR_NO_CONNECTION;
	}
	
	return NSAPI_ERROR_OK;
	
}

void QuectelM66Interface::serial_read_thread_entry() {
	
	wd_log_debug("QuectelM66Interface --> serial_read_thread_entry");
	
	this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_START_SIGNAL);
	this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_STOP_SIGNAL);
	
	do {
	
		this->_readProcessingThread.signal_wait(QUECTEL_M66_PPP_READ_START_SIGNAL);
		wd_log_debug("QuectelM66Interface --> QUECTEL_M66_PPP_READ_START_SIGNAL received");
		this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_START_SIGNAL);
	
		int size;
		do {
		
			if (this->_serialStreamAdapter->read(this->_serialBuffer, &size, QUECTEL_M66_READ_BUFFER_SIZE, 1000) == 0) {
				wd_log_debug("QuectelM66Interface --> %d bytes read, forwarding to pppos", size);
				ppp_pcb *tmp = mbed_get_ppp_pcb();
				pppos_input_tcpip(tmp, this->_serialBuffer, size);	
			}
			
		} while (this->_readProcessingThread.signal_wait(QUECTEL_M66_PPP_READ_STOP_SIGNAL, 0).status != osEventSignal);
		wd_log_debug("QuectelM66Interface --> QUECTEL_M66_PPP_READ_STOP_SIGNAL received");
		this->_readProcessingThread.signal_clr(QUECTEL_M66_PPP_READ_STOP_SIGNAL);
	
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
