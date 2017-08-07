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
	
	void onEdge(bool countEdge);
	void onObservingEdge(void) { onEdge(true); }
	void onIgnoringEdge(void) { onEdge(false); }
	void confirmEdge(bool countEdge, int value);
	
	void onPollingTick(void);
	
	Callback<void(uint16_t)> _irq;
	InterruptIn _interruptIn;
	uint16_t _instanceMetadata;
	int _value;
	int _edgeCounter;
	
	Ticker _ticker;
	Timeout _confirmationDelay;
	IOEventQueue * _queue;
};

#endif