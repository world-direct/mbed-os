#pragma once

#include "AnalogOutManager.h"

#define INVALID_VALUE	-1

class ActorCurrentLoopOut {
public:
	ActorCurrentLoopOut(AnalogOutManager * analogOutManager, int outputIndex);
	~ActorCurrentLoopOut();
	
	float getMinRangeValue(void) { return _minRangeValue; };
	void setMinRangeValue(float value) { _minRangeValue = value; };
	float getMaxRangeValue(void) { return _maxRangeValue; };
	void setMaxRangeValue(float value) { _maxRangeValue = value; };
	float getCurrentCalibrationCoefficient(void) { return _currentCalibrationCoefficient; };
	void setCurrentCalibrationCoefficient(float value) { _currentCalibrationCoefficient = value; };
	
	float getValue(void) { return _value; };
	void setValue(float value);
	
private:
	
	float _minRangeValue = 3.8f;
	float _maxRangeValue = 20.5f;
	float _currentCalibrationCoefficient = 5.549f;

	AnalogOutManager * _analogOutManager;
	int _outputIndex;
	
	volatile float _value = 0;
	
};

