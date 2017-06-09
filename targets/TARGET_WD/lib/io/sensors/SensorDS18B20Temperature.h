#pragma once

#include "OneWire.h"

#define INVALID_VALUE	-1000

class SensorDS18B20Temperature {
public:
	SensorDS18B20Temperature(OneWire * oneWire);
	~SensorDS18B20Temperature();
	
	OW_STATUS_CODE convert_temperature(void); // ids, callback..
//	OW_STATUS_CODE read_temperature(float * buffer, char unit); // currently only c supported
	float getValue(void);
	
private:
	OneWire * _oneWire;
	
	
};

