#include "SensorCurrentLoopIn.h"

SensorCurrentLoopIn::SensorCurrentLoopIn(AnalogInManager * analogInManager, int inputIndex)
	: _inputIndex(inputIndex) {

	this->_analogInManager = analogInManager;
}

SensorCurrentLoopIn::~SensorCurrentLoopIn() {

	delete this->_analogInManager;
	this->_analogInManager = NULL;
	
}

float SensorCurrentLoopIn::getValue(void) {
	
	float conversion_result = this->_analogInManager->getValue(this->_inputIndex) * this->_currentCalibrationCoefficient / 1000;
	
	if (conversion_result < _minRangeValue || conversion_result > _maxRangeValue) {
		return INVALID_VALUE;
	} else {
		return conversion_result;
	}
	
}