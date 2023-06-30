#ifndef ANALOG_SENSOR_H
#define ANALOG_SENSOR_H

#include "smartthing/sensors/Sensor.h"

class AnalogSensor: public Sensor {
    public:
        AnalogSensor(const char * name, int pin): Sensor(name, pin) {
            _valueGenerator = [this]() {
                if (_pin > 0) {
                    return (int) analogRead(_pin);
                }
                return -1;
            };
        };
};

#endif