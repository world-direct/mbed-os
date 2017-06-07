#pragma once

#include "AnalogInManager.h"

#define INVALID_VALUE	-1

class SensorCurrentLoopIn {
public:
	SensorCurrentLoopIn(AnalogInManager * analogInManager, int inputIndex);
	~SensorCurrentLoopIn();
	
	float getMinRangeValue(void) { return _minRangeValue; };
	void setMinRangeValue(float value) { _minRangeValue = value; };
	float getMaxRangeValue(void) { return _maxRangeValue; };
	void setMaxRangeValue(float value) { _maxRangeValue = value; };
	float getCurrentCalibrationCoefficient(void) { return _currentCalibrationCoefficient; };
	void setCurrentCalibrationCoefficient(float value) { _currentCalibrationCoefficient = value; };
	
	float getValue(void);
	
private:
	float _minRangeValue = 3.8f;
	float _maxRangeValue = 20.5f;
	float _currentCalibrationCoefficient = 0.3357f;

	AnalogInManager * _analogInManager;
	int _inputIndex;
};

