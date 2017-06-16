#include "AnalogInManager.h"
#include "Median.h"

AnalogInManager::AnalogInManager(int inputCount, PinName muxSel0, PinName muxSel1, PinName muxSel2, PinName spiMiso, PinName spiSck, PinName spiCs)
	: _inputCount(inputCount), _muxSel0(muxSel0, 0), _muxSel1(muxSel1, 0), _muxSel2(muxSel2, 0), _pinCs(spiCs, 1), _spi(NC, spiMiso, spiSck), _ticker() {
	
	// allocate memory for dynamic buffers
	this->_measurementBuffer = new int *[inputCount];

	for( int i = 0 ; i < inputCount ; i++ ) {
		this->_measurementBuffer[i] = new int[MEASUREMENT_BUFFER_SIZE]();
	}
		
	this->_measurementPosition = new int[inputCount]();
		
	this->read(); // initiate first conversion
	this->_ticker.attach(callback(this, &AnalogInManager::collectMeasurement), ((float) MEASUREMENT_INTERVAL_MS)/1000.0f );
		
}

AnalogInManager::~AnalogInManager() {
	
	// free allocated memory for dynamic buffers
	for( int i = 0 ; i < MEASUREMENT_BUFFER_SIZE ; i++ ) {
		delete [] this->_measurementBuffer[i];
	}
	delete [] this->_measurementBuffer;
	
	delete [] this->_measurementPosition;
}


int AnalogInManager::getValue(int inputIndex) {
	
	if (inputIndex < 0 || inputIndex >= this->_inputCount) return 0;
	
	return Median<int>::compute(this->_measurementBuffer[inputIndex], MEASUREMENT_BUFFER_SIZE);
	
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
	
	// save current input selection (adc should already hold result of input value conversion)
	int inputIndex = _currentInputSelection-1;	
	
	// already select next input as the adc immediately enters the capture state after a read operation
	this->selectNextInput();
	
	// wait for propagation delay
	//	wait_ms(1);	// time is below 2 ns so we can savely skip this step
	
	// read last captured value and enter capture state afterwards
	int adc_val = (int)this->read();
	_measurementBuffer[inputIndex][_measurementPosition[inputIndex]] = adc_val;
	wd_log_debug("AIN read -> input %d, value %d", inputIndex, adc_val);
	
	if (_measurementPosition[inputIndex]++ >= MEASUREMENT_BUFFER_SIZE)
		_measurementPosition[inputIndex] = 0;
	
}