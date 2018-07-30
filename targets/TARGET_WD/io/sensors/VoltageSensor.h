#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

#include "CalibratableSensor.h"

class VoltageSensor : public CalibratableSensor{

    public:

        enum class Unit {
            mV,
            V,
        };

        VoltageSensor(VoltageSensor::Unit unit){
            m_unit = unit;
        }
        
        virtual ~VoltageSensor(){}

        VoltageSensor::Unit getUnit(){
            return m_unit;
        }

    private:

        VoltageSensor::Unit m_unit;

};

#endif