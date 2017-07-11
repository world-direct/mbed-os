/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "SensorDigitalIn.h"


/******************************************************
___________________IMPLEMENTATION______________________
******************************************************/
static void donothing(uint16_t instanceId) {}

SensorDigitalIn::SensorDigitalIn(PinName pin, EdgeSelection edgeSelection, uint16_t instanceMetadata)
	: _interruptIn(pin), _instanceMetadata(instanceMetadata), _ticker(), _queue(&IOEventQueue::getInstance()), _value(0), _edgeCounter(0) {
		
	if (edgeSelection == SensorDigitalIn::None) {
		_ticker.attach(callback(this, &SensorDigitalIn::onPollingTick), 0.05f);
	} else if (edgeSelection == SensorDigitalIn::Rising) {
		_interruptIn.rise(callback(this, &SensorDigitalIn::onObservingEdge));
		_interruptIn.fall(callback(this, &SensorDigitalIn::onIgnoringEdge));
	} else {
		_interruptIn.fall(callback(this, &SensorDigitalIn::onObservingEdge));
		_interruptIn.rise(callback(this, &SensorDigitalIn::onIgnoringEdge));
	}
	
	_interruptIn.enable_irq();
	_irq = donothing;
	
	// set initial value
	this->setValue(_interruptIn.read());
}


SensorDigitalIn::~SensorDigitalIn(){
	
	_interruptIn.disable_irq();
	_ticker.detach();
	
}


void SensorDigitalIn::onEdge(bool countEdge) {
	
	// exit irq context and execute confirmation in event queue thread
	Callback<void(bool, int)> e = _queue->event(callback(this, &SensorDigitalIn::confirmEdge));
	e.call(countEdge, this->_interruptIn.read());
	
}


void SensorDigitalIn::confirmEdge(bool countEdge, int value) {
	
	wait_us(250);	// important: wait in event-queue thread (exit irq)!
	
	// we need to confirm interrupt state here in case of noise, hence the delayed, repeated read operation
	if (value == this->_interruptIn.read()) {
		
		if (countEdge) this->_edgeCounter++;
		
		this->setValue(value);
		
	}
	
}


void SensorDigitalIn::onPollingTick(void) {
	
	int value = this->_interruptIn.read();
	if (value > this->_value) this->_edgeCounter ++;
	this->setValue(value);
	
}


void SensorDigitalIn::attach(mbed::Callback<void(uint16_t)> func) {
	
	if (func){
		_irq = _queue->event(func);
	} else {
		_irq = donothing;
	}
	
}


void SensorDigitalIn::detach(void) {
	
	_irq = donothing;
	
}


void SensorDigitalIn::setValue(int value) {
	
	int prev = this->_value;
	this->_value = value;
	
	if (value != prev) _irq.call(this->_instanceMetadata);
	
}