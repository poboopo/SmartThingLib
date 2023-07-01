#ifndef SENSORS_LIST_H
#define SENSORS_LIST_H

#include <functional>
#include <ArduinoJson.h>
#include <Arduino.h>

namespace Configurable {
    namespace Sensor {
        typedef std::function<int(void)> ValueGeneratorFunction;

        // add units?
        struct Sensor {
            int pin;
            const char * name;
            ValueGeneratorFunction valueGenerator = [](){return -1;};

            Sensor * next;
            Sensor * previous;
        };

        class SensorsList {
            public:
                SensorsList(): _head(nullptr){};
                ~SensorsList();

                void add(const char * name, ValueGeneratorFunction valueGenerator);
                void addDigital(const char * name, int pin);
                void addAnalog(const char * name, int pin);

                DynamicJsonDocument getValues();
                int size() {
                    return _count;
                }
            private:
                Sensor * _head;
                int _count;

                void append(Sensor * sensor);
        };
    }
}

#endif