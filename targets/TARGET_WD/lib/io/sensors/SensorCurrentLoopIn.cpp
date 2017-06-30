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
		return CURRENT_LOOP_INVALID_VALUE;
	} else {
		return conversion_result;
	}
	
}

void SensorCurrentLoopIn::attach(Callback<void(uint16_t)> func) {
	
	if (func){
		this->_analogInManager->attach(this->_inputIndex, func);
	}
	
}

void SensorCurrentLoopIn::detach(void) {
	
	this->_analogInManager->detach(this->_inputIndex);
	
}

float SensorCurrentLoopIn::getValueChangedTolerance(void) {
	
	return (float)((float)this->_analogInManager->getValueChangedTolerance(this->_inputIndex) * this->_currentCalibrationCoefficient / 1000.0f);
	
}

void SensorCurrentLoopIn::setValueChangedTolerance(float value) {
	
	int adcValueTolerance = (int)(value * 1000.0f / this->_currentCalibrationCoefficient);
	this->_analogInManager->setValueChangedTolerance(this->_inputIndex, adcValueTolerance);
	
}