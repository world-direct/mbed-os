/*
 * ubirch#1 M66 Modem AT command parser.
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

#include <cctype>
//#include <targets/TARGET_Freescale/TARGET_KSDK2_MCUS/TARGET_K82F/drivers/fsl_rtc.h>
#include <string>
#include "mbed_debug.h"
#include "QuectelM66CommandCoordinator.h"
#include "SerialStreamAdapter.h"
#include "wd_logging.h"

QuectelM66CommandCoordinator::QuectelM66CommandCoordinator(IOStream* serialStream, PinName pwrKey, PinName vdd_ext, const char *apn, const char *userName, const char *passPhrase)
	: _atCommandInterface(serialStream)
	, _pwrKeyPin(pwrKey)
	, _vdd_extPin(vdd_ext)
{
		
}

QuectelM66CommandCoordinator::~QuectelM66CommandCoordinator() {
	
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
	if (_atCommandInterface.open() != OK) {
		wd_log_error("QuectelM66CommandCoordinator --> Open AT-Interface failed");	
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> Open AT-Interface succeeded");
	// Init ATCommandsInterface
	wd_log_info("QuectelM66CommandCoordinator --> Init AT-Interface");	
	if (_atCommandInterface.init() != OK) {
		wd_log_error("QuectelM66CommandCoordinator --> Init AT-Interface failed");	
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> Init AT-Interface succeeded");
	
	// ====== PPP ========
	// 1.Synchronization between TE and TA 
	// 2. Fix and save baudrate by AT+IPR=xxx&W
	wd_log_info("QuectelM66CommandCoordinator --> \"Fix and save baudrate\"");
	ATCommandsInterface::ATResult result;
	if (_atCommandInterface.executeSimple("AT+IPR=115200&W", &result, 5000, 3) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"Fix and save baudrate\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"Fix and save baudrate\" succeeded");
	
	/*
		Query SIM Card Status: AT+CPIN/AT+QINISTAT. 
		Reboot module if module failed to detect SIM Card in 10s with AT+CPIN?
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"Query SIM Card Status\"");
	if (_atCommandInterface.executeSimple("AT+CPIN?", &result, 2000, 3) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"Query SIM Card Status\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"Query SIM Card Status\" succeeded");
	
	/*
		GSM Network： 
			1. It indicates that module has registered to GSM network when AT+CREG? returns 1 or 5. 
			2. Reboot the module if it failed to register to GSM network in 30s.
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"GSM Network\"");
	if (_atCommandInterface.executeSimple("AT+CREG?", &result, 2000, 3) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"GSM Network\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"GSM Network\" succeeded");
	
	/*
		GPRS Network： 
			1. It indicates that module has registered to GPRS network when AT+CGREG? returns 1 or 5. 
			2. It is able to go to next step without registering to GPRS network in 30s
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"GPRS Network\"");
	if (_atCommandInterface.executeSimple("AT+CGREG?", &result, 2000, 3) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"GPRS Network\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"GPRS Network\" succeeded");
	
	/*
		APN configuration： 
			1. APN must be set by AT+CGDCONT 
			2. Use AT+CGACT? to check whether or not current context has been activated when AT+CGDCONT returns error
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"APN configuration\"");
	//if (_atCommandInterface.executeSimple("AT+CGDCONT=1,\"IP\",\"WD.at-M2M\"", &result, 2000, 3) != 0) {	
	if (_atCommandInterface.executeSimple("AT+CGDCONT=1,\"IP\",\"A1.net\"", &result, 2000, 3) != 0) {	
		wd_log_error("QuectelM66CommandCoordinator --> \"APN configuration\" failed");
		return false;
	};
	wd_log_debug("QuectelM66CommandCoordinator --> \"APN configuration\" succeeded");
	
	/*
		Start PPP Dialling by ATD*99#
	*/
	wd_log_info("QuectelM66CommandCoordinator --> \"Start Dialing\"");
	_atCommandInterface.executeSimple("ATD*99#", &result, 2000, 3);
	if (result.result != ATCommandsInterface::ATResult::AT_CONNECT) {
		wd_log_error("QuectelM66CommandCoordinator --> \"Start Dialing\" failed");
		return false;
	}
	wd_log_debug("QuectelM66CommandCoordinator --> \"Start Dialing\" succeeded");
	
	wd_log_info("QuectelM66CommandCoordinator --> \"Close AT-Interface\"");
	if (_atCommandInterface.close() != 0) {	
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