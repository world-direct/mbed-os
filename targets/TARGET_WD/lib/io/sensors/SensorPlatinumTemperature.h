#pragma once

#include "AnalogInManager.h"

#define INVALID_VALUE	-1

class SensorPlatinumTemperature {
public:
	SensorPlatinumTemperature(AnalogInManager * analogInManager, int inputIndex);
	~SensorPlatinumTemperature();
	
	float getMinRangeValue(void) { return _minRangeValue; };
	void setMinRangeValue(float value) { _minRangeValue = value; };
	float getMaxRangeValue(void) { return _maxRangeValue; };
	void setMaxRangeValue(float value) { _maxRangeValue = value; };
	float getCalibrationCoefficient(void) { return _calibrationCoefficient; };
	void setCalibrationCoefficient(float value) { _calibrationCoefficient = value; };
	
	float getValue(void);
	
private:
	float _minRangeValue = -50.0f; // TODO
	float _maxRangeValue = 100.0f; // TODO
	float _calibrationCoefficient = 1.0f; // TODO

	AnalogInManager * _analogInManager;
	int _inputIndex;
};

