#ifndef POWER_SENSOR_H
#define POWER_SENSOR_H

#include "io/sensors/SensorBase.h"

class PowerSensor : public SensorBase{

    public:

        /**
         * Gets the current calibration coefficient
         */
        virtual float getCalibration() = 0;

        /**
         * Read or Write the current calibration coefficient
         */
        virtual void setCalibration(float value) = 0;    
};

#endif