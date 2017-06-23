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
	
	// TODO: implementation for PT100 -> which factor is different?
	uint64_t adcval = (uint64_t)this->_analogInManager->getValue(this->_inputIndex);
	
	int32_t r = (uint64_t)((uint64_t)((uint64_t)2700 * adcval * (uint64_t)512) / (uint64_t)((uint64_t)0x10000 - adcval));
	int64_t tx100 = (((int64_t)13300 * r) - (int64_t)6809600000) / 0x40000;
	
	float conversion_result = (float)tx100 / 100.0f;
	
	if (this->_unit == SensorPlatinumTemperature::FAHRENHEIT) {
		conversion_result = ((9.0f / 5.0f) * conversion_result) + 32.0f;
	}
	
	if (conversion_result < _minRangeValue || conversion_result > _maxRangeValue) {
		return INVALID_VALUE;
	} else {
		return conversion_result;
	}
	
}