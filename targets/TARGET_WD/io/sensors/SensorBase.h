#ifndef SENSOR_BASE_H
#define SENSOR_BASE_H

#include "mbed.h"

class SensorBase {
    public:
        
        SensorBase();
    
        virtual ~SensorBase(){}
        
        /**
         * Gets the current value of the sensor
         */
        float getValue();

        /**
         * Gets the minimum value measured by the sensor since power ON or reset
         */
        float getMinMeasuredValue();

        /**
         * Gets the maximum absolute measurement tolerance of the sensor
         * Changes smaller this value will be ignored.
         */ 
        float getMaxMeasurementTolerance();

        /** 
         * Sets the maximum absolute measurement tolerance to the provided value
         * Changes smaller this value will be ignored.
         */
        void setMaxMeasurementTolerance(float value);

        /**
         * Gets the maximum value measured by the sensor since power ON or reset
         */
        float getMaxMeasuredValue();

        /**
         * Gets the minimum value that can be measured by the sensor
         */
        float getMinRangeValue();

        /**
         * Gets the maximum value that can be measured by the sensor
         */
        float getMaxRangeValue();

        /**
         * Reset the Min and Max Measured Values to Current Value
         */
        void resetStatistics();

        /**
         * Attaches a callback, which is triggered when the current value of the sensor changes
         */
        void attach(Callback<void(float)> cb);

        /**
         * Removes the value changed callback
         */
        void detach();

    protected:

        /**
         * Sets the current value of the sensor
         */
        void setCurrentValue(float value);

        /**
         * Sets the minimum value that can be measured by the sensor
         */
        void setMinRangeValue(float value);

        /**
         * Sets the maximum value that can be measured by the sensor
         */
        void setMaxRangeValue(float value);

    private:               

        float m_measure_current;
        float m_measure_min;
        float m_measure_max;
        float m_measure_tolerance;
        float m_range_min;
        float m_range_max;
        float m_change_min;
        
        Callback<void(float)> m_measure_changed;
}; 

#endif