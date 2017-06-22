/*
 * WatchDog.h
 *
 * Copyright (C) 2017 world-direct.at, MIT License
 *
 * Created: 21.06.2017
 * Author:	Simon Pfeifhofer
 * EMail:	simon.pfeifhofer@world-direct.at
 *
 * Description:
 *		Provides the watch-dog-functionality (IWDG)
 *
 */ 

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include "mbed.h"
#include "HealthCheckBase.h"

class WatchDog {
	
	private:
	
		IWDG_HandleTypeDef _handle;
	
	public:
	

		WatchDog();
	
		/* 
			Configures the Watchdog
		*/
		void Configure(uint32_t prescaler, uint32_t reload);
	
		/* 
			Resets the counter to avoid a system-reset
		*/
		void Kick();
	
		/*
			Registers a new HealthCheck
		*/
		void RegisterHealthCheck(HealthCheckBase * healthCheck);
	
};

#endif


