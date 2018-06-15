#pragma once
#include "PinNames.h"
#include "mbed.h"

// Helpers
#include "AnalogInManager.h"
#include "AnalogOutManager.h"
#include "OneWire.h"
#include "DS18B20.h"

// Sensors
#include "SensorDigitalIn.h"
#include "SensorCurrentLoopIn.h"
#include "PlatinumTemperaturSensorAim.h"

// Actors
#include "ActorDigitalOutGpio.h"
#include "ActorCurrentLoopOut.h"

#define DINCount	10
#define AINCount	6
#define PTCount		2
#define DOUTCount	3
#define AOUTCount	2

extern struct RoutingmaxIO {
	SensorDigitalIn DINs[DINCount];
	SensorCurrentLoopIn AINs[AINCount];
	PlatinumTemperaturSensorAim PTs[PTCount];
	DS18B20 OneWireDS18B20;
	ActorDigitalOutGpio DOUTs[DOUTCount];
	ActorCurrentLoopOut AOUTs[AOUTCount];
} routingmax_io;

extern uint8_t board_hw_version;