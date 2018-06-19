#ifndef PLATINUM_TEMPERATUR_SENSOR_H
#define PLATINUM_TEMPERATUR_SENSOR_H

#define PT_INVALID_VALUE				-999.99f
#define PT_VALUE_CHANGED_TOLERANCE		0.1f

#include <cstdint>
#include "mbed.h"
#include "SensorBase.h"

class PlatinumTemperaturSensor : public SensorBase {

public:
	enum PTUnit {
		CELSIUS,
		FAHRENHEIT
	};
	
	enum PTType {
		PT100,
		PT1000
	};

	PlatinumTemperaturSensor(PlatinumTemperaturSensor::PTType type);
	
	/**
	 * Gets the unit of the measurements 
	 */
	PTUnit getUnit(void);

	/**
	 * Sets the unit of the measurments
	 */
	void setUnit(PTUnit value);

	/**
	 * Gets the type of the sensor
	 */
	PTType getType(void);

	
protected:
	
	/**
	 * Converts an analog digital converter output into temperatur
	 */
	float adc2temperature(uint16_t adc);
		
	/**
	 * Converts a temperatur into a analog/digital converter output
	 */
	uint16_t temperature2adc(float temp);

private:

	PTUnit m_unit;
	PTType m_ptType;
};

#endif