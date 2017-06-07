#include "SensorPlatinumTemperature.h"

SensorPlatinumTemperature::SensorPlatinumTemperature(AnalogInManager * analogInManager, int inputIndex)
	: _inputIndex(inputIndex) {

	this->_analogInManager = analogInManager;
}

SensorPlatinumTemperature::~SensorPlatinumTemperature() {

	delete this->_analogInManager;
	this->_analogInManager = NULL;
	
}

float SensorPlatinumTemperature::getValue(void) {
	
	float conversion_result = this->_analogInManager->getValue(this->_inputIndex) * this->_calibrationCoefficient;
	
	if (conversion_result < _minRangeValue || conversion_result > _maxRangeValue) {
		return INVALID_VALUE;
	} else {
		return conversion_result;
	}
	
}