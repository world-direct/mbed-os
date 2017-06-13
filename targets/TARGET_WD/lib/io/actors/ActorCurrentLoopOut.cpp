#include "ActorCurrentLoopOut.h"


ActorCurrentLoopOut::ActorCurrentLoopOut(AnalogOutManager * analogOutManager, int outputIndex) 
	: _outputIndex(outputIndex) {

	this->_analogOutManager = analogOutManager;
}


ActorCurrentLoopOut::~ActorCurrentLoopOut() {}


/**
* \brief Set value.
*
* \param value		Value in 4 - 20 mA.
*/
void ActorCurrentLoopOut::setValue(int value) {
	
	if (value < _minRangeValue) {
		value = _minRangeValue;
	} else if (value > _maxRangeValue) {
		value = _maxRangeValue;
	}
	
	this->_value = value;
		
	float dac_value = ((float)(value * 1000.0f)) / this->_currentCalibrationCoefficient;
		
	_analogOutManager->setValue(this->_outputIndex, (int)dac_value);
	
}