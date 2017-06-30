#include "SensorPlatinumTemperature.h"

SensorPlatinumTemperature::SensorPlatinumTemperature(AnalogInManager * analogInManager, int inputIndex, PTType ptType)
	: _inputIndex(inputIndex), _minRangeValue(-273.15f), _maxRangeValue(1000.0f), _unit(SensorPlatinumTemperature::CELSIUS), _ptType(ptType) {

	this->_analogInManager = analogInManager;
}

SensorPlatinumTemperature::~SensorPlatinumTemperature() {

	delete this->_analogInManager;
	this->_analogInManager = NULL;
	
}

float SensorPlatinumTemperature::adc2Temperature(uint16_t adc) {
	
	uint64_t rv = this->_ptType == SensorPlatinumTemperature::PT1000 ? 2700 : 3300;
	
	int32_t resistance = (uint64_t)((uint64_t)(rv * (uint64_t)adc) / (uint64_t)((uint64_t)0x10000 - adc));
	resistance *=  512;
	
	int64_t tx100 = (((int64_t)13300 * resistance) - (int64_t)6809600000) / 0x40000;
	
	float result = (float)tx100 / 100.0f;
	
	if (this->_ptType == SensorPlatinumTemperature::PT100) {
		result /= 19.0f;
	}
	
	if (this->_unit == SensorPlatinumTemperature::FAHRENHEIT) {
		result = ((9.0f / 5.0f) * result) + 32.0f;
	}
	
	return result;
	
}


float SensorPlatinumTemperature::getValue(void) {
	
	float conversion_result = this->adc2Temperature(this->_analogInManager->getValue(this->_inputIndex));
	
	if (conversion_result < this->_minRangeValue || conversion_result > this->_maxRangeValue) {
		return INVALID_VALUE;
	} else {
		return conversion_result;
	}
	
}