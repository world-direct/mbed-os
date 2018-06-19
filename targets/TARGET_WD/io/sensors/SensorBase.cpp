#include "io/sensors/SensorBase.h"

SensorBase::SensorBase(): 
    m_measure_current(NAN),
    m_measure_min(HUGE_VALF),
    m_measure_max(-HUGE_VALF),
    m_measure_tolerance(0.0f),
    m_range_min(-HUGE_VALF),
    m_range_max(HUGE_VALF) {
}

float SensorBase::getValue(){
    return m_measure_current;
}

void SensorBase::setCurrentValue(float value){

    if(value < m_range_min || value > m_range_max || isnan(value)){
        value = NAN;
    } else {
        m_measure_min = value < m_measure_min ? value : m_measure_min;
        m_measure_max = value > m_measure_max ? value : m_measure_max;
    }

    float diff = abs(m_measure_current - value);
    if(diff < m_measure_tolerance) return;

    m_measure_current = value;

    if(m_measure_changed){
        m_measure_changed(m_measure_current);
    }        
}

float SensorBase::getMinMeasuredValue(){
    return m_measure_min;
}

float SensorBase::getMaxMeasuredValue(){
    return m_measure_max;
}

void SensorBase::setMaxMeasurementTolerance(float value){
    m_measure_tolerance = value;
}

float SensorBase::getMinRangeValue(){
    return m_range_min;
}

void SensorBase::setMinRangeValue(float value){
    m_range_min = value;
}

float SensorBase::getMaxRangeValue(){
    return m_range_max;
}

void SensorBase::setMaxRangeValue(float value){
    m_range_max = value;
}

void SensorBase::resetStatistics(){
    m_measure_min = m_measure_max = m_measure_current;
}

void SensorBase::attach(Callback<void(float)> cb){
    m_measure_changed = cb;
}

void SensorBase::detach(){
    m_measure_changed = NULL;
}