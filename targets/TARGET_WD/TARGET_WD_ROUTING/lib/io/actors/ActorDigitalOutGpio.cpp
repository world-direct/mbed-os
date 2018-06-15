/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "ActorDigitalOutGpio.h"


/******************************************************
___________________IMPLEMENTATION______________________
******************************************************/

int ActorDigitalOutGpio::getValue(){
	return this->_value;
}

void ActorDigitalOutGpio::setValue(int value) {
	
	this->_value = value;
	_digitalOut = value;	
}