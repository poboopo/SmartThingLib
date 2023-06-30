#ifndef SENSOR_H
#define SENSOR_H

#include <functional>
#include <Arduino.h>
#include "smartthing/logs/BetterLogger.h"

class Sensor {
    public:
        typedef std::function<int(void)> ValueGeneratorFunction;
        Sensor (const char * name, ValueGeneratorFunction function): 
            _name(name), _valueGenerator(function) {};
        Sensor (const char * name, int pin): _name(name), _pin(pin) {
        };

        int getValue() {
            return _valueGenerator();
        };
        const char * getName() {return _name;};

        Sensor * next;
        Sensor * previous;
    protected:
        const char * _name;
        int _pin = -1;
        ValueGeneratorFunction _valueGenerator = [](){return -1;};
};

#endif
