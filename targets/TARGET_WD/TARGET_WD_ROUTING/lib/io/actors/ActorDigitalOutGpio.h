#ifndef ACTOR_DIGITAL_OUT_GPIO_H
#define ACTOR_DIGITAL_OUT_GPIO_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"
#include "PinNames.h"

/******************************************************
___________________DECLARATION_________________________
******************************************************/
class ActorDigitalOutGpio : public ActorDigitalOut {
public:
	
	ActorDigitalOutGpio(PinName pin, int value = 0) : _digitalOut(pin, value), _value(value){};
	~ActorDigitalOutGpio(){};
	
	int getValue(void) { return _value; };
	void setValue(int value);
	
private:
	DigitalOut _digitalOut;
	int _value;
	
};

#endif