#include "PlatinumTemperaturSensor.h"

PlatinumTemperaturSensor::PlatinumTemperaturSensor(PlatinumTemperaturSensor::PTType type) : 
	SensorBase(), 
	m_ptType(type),
	m_unit(PlatinumTemperaturSensor::PTUnit::CELSIUS){

	setMinRangeValue(-50.0f);
	setMaxRangeValue(150.0f);
	setMaxMeasurementTolerance(PT_VALUE_CHANGED_TOLERANCE);
}

PlatinumTemperaturSensor::PTUnit PlatinumTemperaturSensor::getUnit(){
    return this->m_unit;
}

void PlatinumTemperaturSensor::setUnit(PlatinumTemperaturSensor::PTUnit value){
    this->m_unit = value;
}

PlatinumTemperaturSensor::PTType PlatinumTemperaturSensor::getType(){
    return this->m_ptType;
}

float PlatinumTemperaturSensor::adc2temperature(uint16_t adc) {
	
	uint64_t rv = this->m_ptType == PlatinumTemperaturSensor::PT1000 ? 2700 : 3300;
	
	int32_t resistance = (uint64_t)((uint64_t)(rv * (uint64_t)adc) / (uint64_t)((uint64_t)0x10000 - (uint64_t)adc));
	resistance *=  512;
	
	int64_t tx100 = (((int64_t)13300 * resistance) - (int64_t)6809600000) / 0x40000;
	
	float result = (float)tx100 / 100.0f;
	
	if (this->m_ptType == PlatinumTemperaturSensor::PT100) {
		result /= 19.0f;
	}
	
	if (this->m_unit == PlatinumTemperaturSensor::FAHRENHEIT) {
		result = ((9.0f / 5.0f) * result) + 32.0f;
	}
	
	return result;	
}

uint16_t PlatinumTemperaturSensor::temperature2adc(float temp) {

	int64_t resistance = ((int64_t)(262144 * temp) + (int64_t)68096000) / 133;
	int64_t rv = this->m_ptType == PlatinumTemperaturSensor::PT1000 ? 2700 : 3300;
	uint16_t adc = ((int64_t)(65536 * resistance)) / ((int64_t)512 * rv + resistance);
	return adc;
}