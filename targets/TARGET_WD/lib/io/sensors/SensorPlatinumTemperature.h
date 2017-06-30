#pragma once

#include "AnalogInManager.h"

#define INVALID_VALUE	-999.99f

class SensorPlatinumTemperature {
public:
	enum PTUnit {
		CELSIUS,
		FAHRENHEIT
	};
	
	enum PTType {
		PT100,
		PT1000
	};
	
	SensorPlatinumTemperature(AnalogInManager * analogInManager, int inputIndex, PTType ptType);
	~SensorPlatinumTemperature();
	
	float getMinRangeValue(void) { return _minRangeValue; };
	void setMinRangeValue(float value) { _minRangeValue = value; };
	float getMaxRangeValue(void) { return _maxRangeValue; };
	void setMaxRangeValue(float value) { _maxRangeValue = value; };
	PTUnit getUnit(void) { return _unit; };
	void setUnit(PTUnit value) { _unit = value; };
	
	float getValue(void);
	
private:
	float adc2Temperature(uint16_t adc);
	
	float _minRangeValue; // TODO
	float _maxRangeValue; // TODO
	PTUnit _unit;
	PTType _ptType;
	
	AnalogInManager * _analogInManager;
	int _inputIndex;
};

