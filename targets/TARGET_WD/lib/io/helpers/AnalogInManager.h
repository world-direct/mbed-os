#pragma once

#include "mbed.h"
#include "mbed_events.h"
#include "rtos.h"
#include "PinNames.h"
#include "wd_logging.h"
#include "MeasurementBuffer.h"

#define AIN_MEASUREMENT_BUFFER_SIZE				9
#define AIN_MEASUREMENT_INTERVAL_MS				100	// experienced several faulty reads (0xFF) at an interval of 50ms
#define AIN_DEFAULT_VALUE_CHANGED_TOLERANCE		200

class AnalogInManager {
public:
	AnalogInManager(int inputCount = 0, PinName muxSel0 = NC, PinName muxSel1 = NC, PinName muxSel2 = NC, PinName spiMiso = NC, PinName spiSck = NC, PinName spiCs = NC);
	~AnalogInManager();
	
	// tolerance in respect to ADC value
	int getValueChangedTolerance(int inputIndex) { return _valueChangedTolerance[inputIndex]; };
	void setValueChangedTolerance(int inputIndex, int value) { _valueChangedTolerance[inputIndex] = value; };
	
	void attach(int inputIndex, Callback<void(uint16_t)> func);
	void detach(int inputIndex);
	
	int getValue(int inputIndex);
	
private:
	
	void selectNextInput(void);
	uint16_t read(void);
	void collectMeasurement(void);
	
	int _inputCount;
	int _currentInputSelection = 1;
	
	typedef MeasurementBuffer<uint16_t, AIN_MEASUREMENT_BUFFER_SIZE> AINMeasurementBuffer;
	AINMeasurementBuffer * _measurementBuffers;
	
	uint16_t * _currentValue = 0;
	int * _valueChangedTolerance = 0;
	
	Callback<void(uint16_t)> * _irq;
	
	DigitalOut _muxSel0;
	DigitalOut _muxSel1;
	DigitalOut _muxSel2;
	DigitalOut _pinCs;
	SPI _spi;
	Ticker _ticker;
	
	EventQueue _queue;
	Thread _eventThread;
	
};

