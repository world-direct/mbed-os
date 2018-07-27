#ifndef CALIBRATABLE_SENSOR_H
#define CALIBRATABLE_SENSOR_H

#include "SensorBase.h"
#include <string>

using namespace std;

class CalibratableSensor : public SensorBase{

    public:

        /**
         * Gets the current calibration coefficient
         */
        virtual string getCalibration() = 0;

        /**
         * Read or Write the current calibration coefficient
         * 
         * returns true on success otherwise false
         */
        virtual bool setCalibration(string value) = 0;
  
};

#endif