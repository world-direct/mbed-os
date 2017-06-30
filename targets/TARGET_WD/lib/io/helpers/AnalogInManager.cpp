#include "AnalogInManager.h"
#include <cmath>


static void donothing(uint16_t instanceId) {}

AnalogInManager::AnalogInManager(int inputCount, PinName muxSel0, PinName muxSel1, PinName muxSel2, PinName spiMiso, PinName spiSck, PinName spiCs)
	: _inputCount(inputCount), _muxSel0(muxSel0, 0), _muxSel1(muxSel1, 0), _muxSel2(muxSel2, 0), _pinCs(spiCs, 1), _spi(NC, spiMiso, spiSck), _ticker(), _queue(inputCount * EVENTS_EVENT_SIZE) {
	
	// allocate memory for dynamic buffers
	this->_measurementBuffers = new AINMeasurementBuffer[inputCount]();
	this->_currentValue = new int[inputCount]();
	this->_valueChangedTolerance = new int[inputCount]();
	this->_irq = new Callback<void(uint16_t)>[inputCount]();
		
	// value-change callback handling
	this->_eventThread.start(callback(&_queue, &EventQueue::dispatch_forever));
	for (int i = 0; i < inputCount; i++) {
        _irq[i] = donothing;
		_valueChangedTolerance[i] = AIN_DEFAULT_VALUE_CHANGED_TOLERANCE;
    }

	this->read(); // initiate first conversion
	this->_ticker.attach(callback(this, &AnalogInManager::collectMeasurement), ((float) AIN_MEASUREMENT_INTERVAL_MS)/1000.0f );
		
}


AnalogInManager::~AnalogInManager() {
	
	// free allocated memory for dynamic buffers
	delete [] this->_measurementBuffers;
	delete [] this->_currentValue;
	delete [] this->_valueChangedTolerance;
	delete [] this->_irq;
}


void AnalogInManager::attach(int inputIndex, Callback<void(uint16_t)> func) {
	
	if (inputIndex < 0 || inputIndex >= this->_inputCount) return;
	
	if (func){
		_irq[inputIndex] = _queue.event(func);
	} else {
		_irq[inputIndex] = donothing;
	}
	
}


void AnalogInManager::detach(int inputIndex) {
	
	_irq[inputIndex] = donothing;
	
}


int AnalogInManager::getValue(int inputIndex) {
	
	if (inputIndex < 0 || inputIndex >= this->_inputCount) return 0;
	
	return this->_currentValue[inputIndex];
	
}


void AnalogInManager::selectNextInput(void) {
	
	_muxSel0 = !_muxSel0;
	
	if (_currentInputSelection % 2 == 0) _muxSel1 = !_muxSel1;
	
	if (_currentInputSelection % 4 == 0) _muxSel2 = !_muxSel2;
	
	if (++_currentInputSelection > this->_inputCount) { // reset
		_currentInputSelection = 1;
		_muxSel0 = 0;
		_muxSel1 = 0;
		_muxSel2 = 0;
	}
	
	wd_log_debug("AIN MUX select %d %d %d -> input %d", _muxSel2.read(), _muxSel1.read(), _muxSel0.read(), _currentInputSelection);
	
}


uint16_t AnalogInManager::read(void) {
	
	this->_pinCs = 0;
	
	int hb = _spi.write(0xFF);
	int lb = _spi.write(0xFF);
	
	this->_pinCs = 1;
		
	return (((uint16_t) hb) << 8) | ((uint16_t) lb);
	
}


void AnalogInManager::collectMeasurement(void) {
	
	// save current input selection (adc should already hold result of input value conversion, i.e. value of previously selected input)
	int inputIndex = _currentInputSelection-1;	
	
	// already select next input as the adc immediately enters the capture state after a read operation
	this->selectNextInput();
	
	// wait for propagation delay
	//	wait_ms(1);	// time is below 2 ns so we can savely skip this step
	
	// read last captured value and enter capture state afterwards
	uint16_t adc_val = this->read();
	_measurementBuffers[inputIndex].add(adc_val);
	wd_log_debug("AIN read -> input %d, value %d", inputIndex, adc_val);
	
	// detect value change
	uint16_t previousValue = this->_currentValue[inputIndex];
	this->_currentValue[inputIndex] = this->_measurementBuffers[inputIndex].get();
	
	if (abs(previousValue - this->_currentValue[inputIndex]) > this->_valueChangedTolerance[inputIndex]) {
		_irq->call(inputIndex);
	}
	
}