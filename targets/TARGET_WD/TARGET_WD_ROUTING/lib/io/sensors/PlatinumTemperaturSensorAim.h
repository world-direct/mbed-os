#ifndef PLATINUM_TEMPERATUR_SENSOR_AIM_H
#define PLATINUM_TEMPERATUR_SENSOR_AIM_H

#include "PlatinumTemperaturSensor.h"
#include "AnalogInManager.h"

/**
 * Represents an platinum temperature sensor (PT100/PT1000) connected to an analog in manager
 */
class PlatinumTemperaturSensorAim : public PlatinumTemperaturSensor {
public:
	
	PlatinumTemperaturSensorAim(AnalogInManager * analogInManager, int inputIndex, PlatinumTemperaturSensor::PTType ptType);
	~PlatinumTemperaturSensorAim();	
	
private:
	
	void sensorValueChanged(uint16_t input);
	AnalogInManager * _analogInManager;
	int _inputIndex;
};

#endif