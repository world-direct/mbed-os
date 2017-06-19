#pragma once

#include "mbed.h"
#include "PinNames.h"
#include "wd_logging.h"

#define MEASUREMENT_BUFFER_SIZE		9
#define MEASUREMENT_INTERVAL_MS		100	// experienced several faulty reads (0xFF) at an interval of 50ms

class AnalogInManager {
public:
	AnalogInManager(int inputCount = 0, PinName muxSel0 = NC, PinName muxSel1 = NC, PinName muxSel2 = NC, PinName spiMiso = NC, PinName spiSck = NC, PinName spiCs = NC);
	~AnalogInManager();
	
	int getValue(int inputIndex);
	
private:
	
	void selectNextInput(void);
	uint16_t read(void);
	void collectMeasurement(void);
	
	int _inputCount;
	int _currentInputSelection = 1;
	
	int** _measurementBuffer = 0;
	int * _measurementPosition = 0;
	
	DigitalOut _muxSel0;
	DigitalOut _muxSel1;
	DigitalOut _muxSel2;
	DigitalOut _pinCs;
	SPI _spi;
	Ticker _ticker;
	
};

