#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include "CalibratableSensor.h"

class CurrentSensor : public CalibratableSensor{

    public:

        enum class Unit {
            mA,
            A,
        };

        CurrentSensor(CurrentSensor::Unit unit){
            m_unit = unit;
        }
        
        virtual ~CurrentSensor(){}

        CurrentSensor::Unit getUnit(){
            return m_unit;
        }

    private:

        CurrentSensor::Unit m_unit;

};

#endif