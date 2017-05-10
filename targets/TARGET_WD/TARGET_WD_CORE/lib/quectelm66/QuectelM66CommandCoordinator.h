/*!
 * @file
 * @brief AT command parser for M66 modem.
 *
 * Contains functions for reading and writing from
 * the serial port
 *
 * @author Niranjan Rao
 * @date 2017-02-09
 *
 * @copyright &copy; 2015, 2016, 2017 ubirch GmbH (https://ubirch.com)
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
#ifndef QUECTELM66COORDINATOR_H
#define QUECTELM66COORDINATOR_H

#include "mbed.h"
#include <stdint.h>
#include <features/netsocket/nsapi_types.h>
#include "Cellular/at/ATCommandsInterface.h"

/** 
	Coordinates power-sequence, AT-commands and PPP-connection
 */
class QuectelM66CommandCoordinator {
public:
	QuectelM66CommandCoordinator(IOStream* serialStream, PinName pwrKey, PinName vdd_ext, const char *apn, const char *userName, const char *passPhrase);

	virtual ~QuectelM66CommandCoordinator();
	
    /**
    * Startup the M66
    *
    * @return true only if M66 was started correctly
    */
	bool startup();
	
	/**
    * Shutdown the M66
    *
    * @return true only if M66 was shutdown correctly
    */
	bool shutdown();

    /**
    * Reset M66
    *
    * @return true only if M66 resets successfully
    */
    bool reset();

private:
    ATCommandsInterface _atCommandInterface;
	DigitalOut _pwrKeyPin;
	DigitalIn _vdd_extPin;
	
	bool pppPreparation();
	
};

#endif // QUECTELM66COORDINATOR_H
