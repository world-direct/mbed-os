/*
 * QuectelM66CommandCoordinator.cpp
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created: 21.04.2017
 * Author:	Simon Pfeifhofer
 * EMail:	simon.pfeifhofer@world-direct.at
 *
 * Description:
 *	 Contains a coordinator which
 *		(a) encapsulates the startup-sequence for the modem and 
 *		(b) starts the PPP-communication by dialling.
 *
 */ 

#include <cctype>
#include <string>
#include "QuectelM66CommandCoordinator.h"
#include "SerialStreamAdapter.h"
#include "wd_logging.h"

QuectelM66CommandCoordinator::QuectelM66CommandCoordinator(IOStream* serialStream, PinName pwrKey, PinName vdd_ext, const char *apn)
	: QuectelM66CommandCoordinator(
		new ATCommandsInterface(serialStream), 
		pwrKey,
		vdd_ext,
		apn){	
}

QuectelM66CommandCoordinator::QuectelM66CommandCoordinator(ATCommandsInterface* atCommandsInterface, PinName pwrKey, PinName vdd_ext, const char *apn):
	_pwrKeyPin(pwrKey)
	, _vdd_extPin(vdd_ext)
	, _apn(apn){
	
		this->_atCommandInterface = atCommandsInterface;
		this->_linkMonitor = new LinkMonitor(atCommandsInterface);
		
}

QuectelM66CommandCoordinator::~QuectelM66CommandCoordinator() {
	
	delete _linkMonitor;
	delete _atCommandInterface;
	
}

bool QuectelM66CommandCoordinator::pppPreparation() {
	
	wd_log_debug("QuectelM66CommandCoordinator --> pppPreparation");
	
	// ====== PPP ========
	// "3 Procedure for PPP Setup" 
	// http://www.quectel.com/UploadImage/Downlad/Quectel_GSM_PPP_Application_Note_V1.1.pdf
	// AT-Commands can be found at
	// http://www.quectel.com/UploadImage/Downlad/Quectel_M66_Series_AT_Commands_Manual_V2.1.pdf
	
	// ====== PPP ========
	// Power On the Module
	// 1. Power OFF : Pull Power Key from high to low, then cut off power after 12s. 
	// 2. Power ON : Pull Power Key to low within 1s. 
	// 3. Pull Power Key back to high when finishing Power OFF or Power ON.
	wd_log_info("QuectelM66CommandCoordinator --> Power On the Module");
	_pwrKeyPin = 0;
	wait_ms(1000);
	_pwrKeyPin = 1;
	wait_ms(100);
	_pwrKeyPin = 0;
	do {
		wd_log_info("QuectelM66CommandCoordinator --> Waiting until VDD_EXT signals");
		wait_ms(100);	
	} while (_vdd_extPin == 0);
	wait_ms(1000);
	_pwrKeyPin = 1;
	wd_log_debug("QuectelM66CommandCoordinator --> Waiting until VDD_EXT signals succeeded");
	
	// ====== AT ========
	// Open ATCommandsInterface
	wd_log_info("QuectelM66CommandCoordinator --> Open AT-Interface");	
	if (_atCommandInterface->open() != OK) {
		wd_log_error("QuectelM66CommandCoordinator --> Open AT-Interface failed");	
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> Open AT-Interface succeeded");
	// Init ATCommandsInterface
	wd_log_info("QuectelM66CommandCoordinator --> Init AT-Interface");	
	if (_atCommandInterface->init() != OK) {
		wd_log_error("QuectelM66CommandCoordinator --> Init AT-Interface failed");	
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> Init AT-Interface succeeded");
	
	// ====== PPP ========
	// 1.Synchronization between TE and TA 
	// 2. Fix and save baudrate by AT+IPR=xxx&W
	wd_log_info("QuectelM66CommandCoordinator --> \"Fix and save baudrate\"");
	ATCommandsInterface::ATResult result;
	if (_atCommandInterface->executeSimple("AT+IPR=115200&W", &result, 1000, 5) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"Fix and save baudrate\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"Fix and save baudrate\" succeeded");
	
	wd_log_info("ATCommandsInterface --> Reset PPP");
	if (_atCommandInterface->executeSimple("ATH", &result, 1000, 3) != OK) {
		wd_log_warn("ATCommandsInterface --> Reset PPP failed, continue anyway");
	}
	wd_log_debug("ATCommandsInterface --> Reset PPP succeeded");
	
	/*
		Query SIM Card Status: AT+CPIN/AT+QINISTAT. 
		Reboot module if module failed to detect SIM Card in 10s with AT+CPIN?
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"Query SIM Card Status\"");
	if (_atCommandInterface->executeSimple("AT+CPIN?", &result, 1000, 10) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"Query SIM Card Status\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"Query SIM Card Status\" succeeded");
	
	/*
		GSM Network： 
			1. It indicates that module has registered to GSM network when AT+CREG? returns 1 or 5. 
			2. Reboot the module if it failed to register to GSM network in 30s.
			
		GPRS Network： 
			1. It indicates that module has registered to GPRS network when AT+CGREG? returns 1 or 5. 
			2. It is able to go to next step without registering to GPRS network in 30s
			
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"GSM Network\" and \"GPRS Network\"");
	
	wd_log_info("QuectelM66CommandCoordinator --> Init LinkMonitor");
	if (_linkMonitor->Init() != OK) {
		wd_log_error("QuectelM66CommandCoordinator --> LinkMonitor initialization failed");	
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> LinkMonitor initialization succeeded");	
	
	bool registered;
	int tries = 10;
	do {
		
		registered = true;
		
		wd_log_info("QuectelM66CommandCoordinator --> LinkMonitor getState (remaining tries %d)", tries);
		if (_linkMonitor->GetState(&_rssi, &_gsmRegistrationState, &_gprsRegistrationState, &_bearer) != OK) {
			wd_log_error("QuectelM66CommandCoordinator --> LinkMonitor getState failed");	
			return false;
		}
		wd_log_debug("QuectelM66CommandCoordinator --> LinkMonitor getState succeeded");	
	
		wd_log_info("QuectelM66CommandCoordinator --> Check GSM registration state (HOME_NETWORK (%d) and ROAMING (%d) are allowed)", 
			LinkMonitor::REGISTRATION_STATE_HOME_NETWORK, 
			LinkMonitor::REGISTRATION_STATE_ROAMING);
		if (_gsmRegistrationState != LinkMonitor::REGISTRATION_STATE_HOME_NETWORK && 
			_gsmRegistrationState != LinkMonitor::REGISTRATION_STATE_ROAMING) {
			wd_log_error("QuectelM66CommandCoordinator --> Invalid GSM registration state (%d)", _gsmRegistrationState);
			registered = false;
		}
		
		wd_log_info("QuectelM66CommandCoordinator --> Check GPRS registration state (HOME_NETWORK (%d) and ROAMING (%d) are allowed)", 
			LinkMonitor::REGISTRATION_STATE_HOME_NETWORK, 
			LinkMonitor::REGISTRATION_STATE_ROAMING);
			
		if (_gprsRegistrationState != LinkMonitor::REGISTRATION_STATE_HOME_NETWORK && 
			_gprsRegistrationState != LinkMonitor::REGISTRATION_STATE_ROAMING) {
			wd_log_error("QuectelM66CommandCoordinator --> Invalid GPRS registration state (%d)", _gprsRegistrationState);
			registered =  false;
		}
		
		tries--;
		
		wait_ms(1000);
		
	} while (!registered && tries > 0);
		
	if (tries <= 0) {
		wd_log_error("QuectelM66CommandCoordinator --> Invalid GSM (%d) and/or GPRS (%d) registration state. GSM (%d)", _gsmRegistrationState, _gprsRegistrationState);
		return false;
	}
	
	wd_log_debug("QuectelM66CommandCoordinator --> Check GSM and GPRS registration succeeded");
	
	wd_log_info("QuectelM66CommandCoordinator --> Check network bearer (BEARER_GSM (%d) and BEARER_UNKNOWN (%d) allowed))", LinkMonitor::BEARER_GSM, LinkMonitor::BEARER_UNKNOWN);
	if (_bearer != LinkMonitor::BEARER_GSM && _bearer != LinkMonitor::BEARER_UNKNOWN) {
		wd_log_error("QuectelM66CommandCoordinator --> Invalid network bearer (%d)", _bearer);
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> Check network bearer succeeded");
	
	/*
		APN configuration： 
			1. APN must be set by AT+CGDCONT 
			2. Use AT+CGACT? to check whether or not current context has been activated when AT+CGDCONT returns error
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"APN configuration\"");
	
	string apn_config_command("AT+CGDCONT=1,\"IP\",");
	apn_config_command += "\"";
	apn_config_command += this->_apn;
	apn_config_command += "\"";
	if (_atCommandInterface->executeSimple(apn_config_command.c_str(), &result, 1000, 3) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"APN configuration\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"APN configuration\" succeeded");
	
	wd_log_info("QuectelM66CommandCoordinator --> \"Get phone number\"");
	if (this->_linkMonitor->GetPhoneNumber(this->_phoneNumber) != 0) {
		wd_log_error("QuectelM66CommandCoordinator --> \"Get phone number\" failed");	
	}
	wd_log_debug("QuectelM66CommandCoordinator --> \"Get phone number\" succeeded");
	
	/*
		Start PPP Dialling by ATD*99#
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"Start Dialing\"");
	_atCommandInterface->executeSimple("ATD*99#", &result, 1000, 3);
	if (result.result != ATCommandsInterface::ATResult::AT_CONNECT) {
		wd_log_error("QuectelM66CommandCoordinator --> \"Start Dialing\" failed");
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> \"Start Dialing\" succeeded");
	
	wd_log_info("QuectelM66CommandCoordinator --> \"Close AT-Interface\"");
	if (_atCommandInterface->close() != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"Close AT-Interface\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"Close AT-Interface\" succeeded");
	
	return true;
	
}

bool QuectelM66CommandCoordinator::startup() {
	
	wd_log_debug("QuectelM66CommandCoordinator --> startup");
	
	if (!this->pppPreparation()) {
		return false;
	}
	
    return true;
}

bool QuectelM66CommandCoordinator::shutdown() {
	
	wd_log_debug("QuectelM66CommandCoordinator --> shutdown");
	// TODO: release serial
	return true;
	
}

bool QuectelM66CommandCoordinator::reset(void) {
	
	wd_log_debug("QuectelM66CommandCoordinator --> reset");
	// TODO
	return true;
    
}

LinkMonitor::REGISTRATION_STATE QuectelM66CommandCoordinator::GetGSMRegistrationState() {
	return this->_gsmRegistrationState;
}

LinkMonitor::REGISTRATION_STATE QuectelM66CommandCoordinator::GetGPRSRegistrationState() {
	return this->_gprsRegistrationState;
}
	
LinkMonitor::BEARER QuectelM66CommandCoordinator::GetBearer() {
	return this->_bearer;
}
	
int QuectelM66CommandCoordinator::GetRSSI() {
	return this->_rssi;
}

char* QuectelM66CommandCoordinator::GetPhoneNumber() {
	return this->_phoneNumber;
}