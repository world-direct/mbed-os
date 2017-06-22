/*
 * WatchDog.cpp
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

#include "WatchDog.h"

WatchDog::WatchDog(){
	
	
}


void WatchDog::Configure(uint32_t prescaler, uint32_t reload) {
	
	_handle.Instance = IWDG;
	_handle.Init.Prescaler = prescaler; // e.g.: IWDG_PRESCALER_256
	_handle.Init.Reload = reload;		// Range: 0x0000, 0x0FFFU
	
	HAL_IWDG_Init(&_handle);
	
}


void WatchDog::Kick() {
	
	HAL_IWDG_Refresh(&_handle);
	
}

void WatchDog::RegisterHealthCheck(HealthCheckBase * healthCheck) {
	
}
