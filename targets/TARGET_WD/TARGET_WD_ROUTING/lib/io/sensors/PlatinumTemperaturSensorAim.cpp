#include "PlatinumTemperaturSensorAim.h"
#include <cmath>

PlatinumTemperaturSensorAim::PlatinumTemperaturSensorAim(AnalogInManager * analogInManager, int inputIndex, PTType ptType)
	: PlatinumTemperaturSensor(ptType), _inputIndex(inputIndex) {

	this->_analogInManager = analogInManager;
	this->setValueChangedTolerance();

	this->_analogInManager->attach(_inputIndex, callback(this, &PlatinumTemperaturSensorAim::sensorValueChanged));
}

PlatinumTemperaturSensorAim::~PlatinumTemperaturSensorAim() {

	this->_analogInManager = NULL;	
}

void PlatinumTemperaturSensorAim::sensorValueChanged(uint16_t input){
	if(_inputIndex != input) return;

	float currentValue = this->adc2temperature(this->_analogInManager->getValue(this->_inputIndex));
	setValue(currentValue);
}