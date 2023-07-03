#ifndef SENSORS_LIST_H
#define SENSORS_LIST_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include "smartthing/configurable/ConfigurableObjects.h"

namespace Configurable {
    namespace Sensor {
        class SensorsList {
            public:
                SensorsList(): _head(nullptr){};
                ~SensorsList();

                bool add(const char * name, ValueGeneratorFunction valueGenerator);
                bool addDigital(const char * name, int pin);
                bool addAnalog(const char * name, int pin);

                DynamicJsonDocument getValues();
                int size() {
                    return _count;
                }
                const Sensor * findSensor(const char * name) const;
            private:
                Sensor * _head;
                int _count;

                bool add(const char * name, SensorType type, ValueGeneratorFunction valueGenerator);
                void append(Sensor * sensor);
        };
    }
}

#endif