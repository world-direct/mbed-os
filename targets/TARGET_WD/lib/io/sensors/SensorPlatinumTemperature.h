#pragma once

#include "AnalogInManager.h"

#define INVALID_VALUE	-1000

class SensorPlatinumTemperature {
public:
	enum PTUnit {
		CELSIUS,
		FAHRENHEIT
	};
	
	SensorPlatinumTemperature(AnalogInManager * analogInManager, int inputIndex);
	~SensorPlatinumTemperature();
	
	float getMinRangeValue(void) { return _minRangeValue; };
	void setMinRangeValue(float value) { _minRangeValue = value; };
	float getMaxRangeValue(void) { return _maxRangeValue; };
	void setMaxRangeValue(float value) { _maxRangeValue = value; };
	PTUnit getUnit(void) { return _unit; };
	void setUnit(PTUnit value) { _unit = value; };
	
	float getValue(void);
	
private:
	float _minRangeValue = -273.15f; // TODO
	float _maxRangeValue = 1000.0f; // TODO
	PTUnit _unit = SensorPlatinumTemperature::CELSIUS;

	AnalogInManager * _analogInManager;
	int _inputIndex;
};

