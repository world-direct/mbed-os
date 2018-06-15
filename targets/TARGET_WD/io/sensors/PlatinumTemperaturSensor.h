#ifndef PLATINUM_TEMPERATUR_SENSOR_H
#define PLATINUM_TEMPERATUR_SENSOR_H

#define PT_INVALID_VALUE				-999.99f
#define PT_VALUE_CHANGED_TOLERANCE		0.1f

#include <cstdint>
#include "mbed.h"

class PlatinumTemperaturSensor {

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
	
	virtual ~PlatinumTemperaturSensor(){}
	
	/** 
	 * Gets the minimum temperatur which can be measured with this sensor
	 */ 
	float getMinRangeValue(void);

	/**
	 * Sets the minimum temperatur that can be measured with this sensor
	 */
	void setMinRangeValue(float value);
	
	/** 
	 * Gets the maximum temperatur which can be measured with this sensor
	 */ 
	float getMaxRangeValue(void);

	/**
	 * Sets the maximum temperatur that can be measured with this sensor
	 */
	void setMaxRangeValue(float value);

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

	/**
	 * Gets the current measurement of the sensor
	 */
	virtual float getValue(void);

	/**
	 * Gets the minimum measured value of the sensor since startup or last reset
	 */
	virtual float getMinMeasuredValue(void);

	/**
	 * Gets the maximum measured value of the sensor since startup or last reset
	 */
	virtual float getMaxMeasuredValue(void);

	/**
	 * Resets the min/max measured valued of this sensor
	 */
	virtual void resetStatistics(void);

	/**
	 * Attaches an callback, which is called when the current value of the sensor has changed
	 */
	virtual void attach(Callback<void(float)> cb);

	/**
	 * Clears a previous attached callback
	 */
	virtual void detach();
	
	/**
	 * Gets the minimum change in the sensor value that triggers a notification
	 */
	virtual float getValueChangedTolerance(void);
	
	/**
	 * Sets the minimum change in the sensor value that triggers a notification
	 */
	virtual void setValueChangedTolerance(float value = PT_VALUE_CHANGED_TOLERANCE);
	
protected:

	/**
	 * Sets the current value of the sensor
	 */
	virtual void setValue(float value);
	
	/**
	 * Converts an analog digital converter output into temperatur
	 */
	float adc2temperature(uint16_t adc);
		
	/**
	 * Converts a temperatur into a analog/digital converter output
	 */
	uint16_t temperature2adc(float temp);

private:

	
	float m_range_min;
	float m_range_max;
	float m_measure_current;
	float m_measure_min;
	float m_measure_max;

	float m_change_min;
	Callback<void(float)> m_measure_current_changed;

	PTUnit m_unit;
	PTType m_ptType;
};

#endif