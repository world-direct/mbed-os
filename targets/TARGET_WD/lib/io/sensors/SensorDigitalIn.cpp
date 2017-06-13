/******************************************************
___________________INCLUDES____________________________
******************************************************/
#include "SensorDigitalIn.h"


/******************************************************
___________________IMPLEMENTATION______________________
******************************************************/
static void donothing(uint16_t instanceId) {}

SensorDigitalIn::SensorDigitalIn(PinName pin, EdgeSelection edgeSelection, uint16_t instanceMetadata)
	: _interruptIn(pin), _instanceMetadata(instanceMetadata) {
	
	// start event queue dispatch thread
	this->_eventThread.start(callback(&_queue, &EventQueue::dispatch_forever));
		
	if (edgeSelection == SensorDigitalIn::Rising) {
		_interruptIn.rise(callback(this, &SensorDigitalIn::onObservingEdge));
		_interruptIn.fall(callback(this, &SensorDigitalIn::onIgnoringEdge));
	} else {
		_interruptIn.fall(callback(this, &SensorDigitalIn::onObservingEdge));
		_interruptIn.rise(callback(this, &SensorDigitalIn::onIgnoringEdge));
	}
	
	_interruptIn.enable_irq();
	_irq.attach(donothing);
}


SensorDigitalIn::~SensorDigitalIn(){
	
	_interruptIn.disable_irq();
	
}


void SensorDigitalIn::onObservingEdge(void) {
	
	this->_edgeCounter++;
	this->setValue(_interruptIn.read());
	
}

void SensorDigitalIn::onIgnoringEdge(void) {
	
	this->setValue(_interruptIn.read());
	
}


void SensorDigitalIn::attach(mbed::Callback<void(uint16_t)> func) {
	
	if (func){
		_irq.attach(_queue.event(func));
	} else {
		_irq.attach(donothing);
	}
	
}


void SensorDigitalIn::detach(void) {
	
	_irq.attach(donothing);
	
}


void SensorDigitalIn::setValue(int value) {
	
	int prev = this->_value;
	this->_value = value;
	
	if (value != prev) _irq.call(this->_instanceMetadata);
	
}


