#ifndef DIGITAL_SENSOR_H
#define DIGITAL_SENSOR_H

#include "smartthing/sensors/Sensor.h"

class DigitalSensor: public Sensor {
    public:
        DigitalSensor(const char * name, int pin): Sensor(name, pin) {
            _valueGenerator = [this]() {
                if (_pin > 0) {
                    return digitalRead(_pin);
                }
                return -1;
            };
        };
};

#endif