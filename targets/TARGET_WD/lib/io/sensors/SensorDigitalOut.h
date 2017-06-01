#ifndef SENSORDIGITALOUT_H
#define SENSORDIGITALOUT_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"
#include "PinNames.h"

/******************************************************
___________________DECLARATION_________________________
******************************************************/
class SensorDigitalOut {
public:
	
	SensorDigitalOut(PinName pin, int value = 0) : _digitalOut(pin, value), _value(value){};
	~SensorDigitalOut(){};
	
	int getValue(void) { return _value; };
	void setValue(int value);
	
private:
	DigitalOut _digitalOut;
	volatile int _value = 0;
	
};

#endif