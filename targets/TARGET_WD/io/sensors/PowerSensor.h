#ifndef POWER_SENSOR_H
#define POWER_SENSOR_H

#include "CalibratableSensor.h"


class PowerSensor : public CalibratableSensor{

    public:
        
        enum class Unit {
            W,
            kW
        };

        PowerSensor(PowerSensor::Unit unit){
            m_unit = unit;
        }

        virtual ~PowerSensor(){}

        PowerSensor::Unit getUnit(){
            return m_unit;
        }

    private:

        PowerSensor::Unit m_unit;
};

#endif