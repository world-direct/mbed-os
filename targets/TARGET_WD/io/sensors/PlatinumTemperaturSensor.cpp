#include "PlatinumTemperaturSensor.h"

PlatinumTemperaturSensor::PlatinumTemperaturSensor(PlatinumTemperaturSensor::PTType type){
	m_ptType = type;
	m_unit = PlatinumTemperaturSensor::PTUnit::CELSIUS;
	m_range_max = 150.0f;
	m_range_min = -50.0f;
	m_change_min = PT_VALUE_CHANGED_TOLERANCE;
	
	resetStatistics();
	setValue(0);
}

float PlatinumTemperaturSensor::getMinRangeValue(){
    return this->m_range_min;
}

void PlatinumTemperaturSensor::setMinRangeValue(float value){
    this->m_range_min = value;
}

float PlatinumTemperaturSensor::getMaxRangeValue(){
    return this->m_range_max;
}

void PlatinumTemperaturSensor::setMaxRangeValue(float value){
    this->m_range_max = value;
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

float PlatinumTemperaturSensor::getMinMeasuredValue(){
    return m_measure_min;
}

float PlatinumTemperaturSensor::getMaxMeasuredValue(){
    return m_measure_max;
}

void PlatinumTemperaturSensor::resetStatistics(){
    this->m_measure_min = 0;
    this->m_measure_max = 0;
}

void PlatinumTemperaturSensor::attach(Callback<void(float)> cb){
	this->m_measure_current_changed = cb;
}

void PlatinumTemperaturSensor::detach(){
	this->m_measure_current_changed = NULL;
}

float PlatinumTemperaturSensor::getValueChangedTolerance(){
	return this->m_change_min;
}

void PlatinumTemperaturSensor::setValueChangedTolerance(float value){
	this->m_change_min = value;
}

float PlatinumTemperaturSensor::getValue(){
	return m_measure_current;
}

void PlatinumTemperaturSensor::setValue(float value){

	
	if (value < m_range_min || value > m_range_max) {
		value = PT_INVALID_VALUE;
	} else {

		if(value < m_measure_min) { 
			m_measure_min = value;
		}
		
		if(value > m_measure_max) {
			m_measure_max = value;
		}
	}

	if(abs(value - m_measure_current) > m_change_min){
		
		m_measure_current = value;
		if(m_measure_current_changed) m_measure_current_changed(value);
	}
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