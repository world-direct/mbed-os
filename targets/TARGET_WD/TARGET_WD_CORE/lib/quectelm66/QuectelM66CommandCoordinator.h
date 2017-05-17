/*
 * QuectelM66CommandCoordinator.h
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
	QuectelM66CommandCoordinator(IOStream* serialStream, PinName pwrKey, PinName vdd_ext, const char *apn);

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
	
	const char* _apn;
	
    ATCommandsInterface _atCommandInterface;
	DigitalOut _pwrKeyPin;
	DigitalIn _vdd_extPin;
	
	bool pppPreparation();
	
};

#endif // QUECTELM66COORDINATOR_H
