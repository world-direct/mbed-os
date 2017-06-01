#ifndef SENSORDIGITALIN_H
#define SENSORDIGITALIN_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"
#include "PinNames.h"

/******************************************************
___________________DECLARATION_________________________
******************************************************/
class SensorDigitalIn {
public:
	enum EdgeSelection {
        Falling = 1,
        Rising = 2
    };
	
	SensorDigitalIn(PinName pin, EdgeSelection edgeSelection = Rising);
	~SensorDigitalIn();
	
	void attach(mbed::Callback<void()> func);
	void detach(void);
	int getValue(void) { return _value; };
	int getCount(void) { return _edgeCounter; };
	void resetCount(void) { _edgeCounter = 0; };
	
private:
	void setValue(int value);
	void onObservingEdge(void);
	void onIgnoringEdge(void);
	
	Callback<void()> _irq;
	InterruptIn _interruptIn;
	volatile int _value = 0;
	volatile int _edgeCounter = 0;
	
};

#endif