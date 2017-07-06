#ifndef SENSORDIGITALIN_H
#define SENSORDIGITALIN_H

/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "mbed.h"
#include "PinNames.h"
#include "IOEventQueue.h"

/******************************************************
___________________DECLARATION_________________________
******************************************************/
class SensorDigitalIn {
public:
	enum EdgeSelection {
        Falling = 1,
        Rising = 2,
		None	// Polling instead of interrupt
    };
	
	SensorDigitalIn(PinName pin, EdgeSelection edgeSelection = Rising, uint16_t instanceMetadata = 0);
	~SensorDigitalIn();
	
	void attach(mbed::Callback<void(uint16_t)> func);
	void detach(void);
	int getValue(void) { return _value; };
	int getCount(void) { return _edgeCounter; };
	void resetCount(void) { _edgeCounter = 0; };
	
private:
	void setValue(int value);
	void onObservingEdge(void);
	void onIgnoringEdge(void);
	void onPollingTick(void);
	
	Callback<void(uint16_t)> _irq;
	InterruptIn _interruptIn;
	uint16_t _instanceMetadata;
	volatile int _value = 0;
	volatile int _edgeCounter = 0;
	
	Ticker _ticker;
	IOEventQueue * _queue;
};

#endif