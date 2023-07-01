#ifndef SENSORS_LIST_H
#define SENSORS_LIST_H

#include <functional>
#include <ArduinoJson.h>
#include <Arduino.h>

namespace Configurable {
    namespace Sensor {
        enum SensorType {
            TYPE_CUSTOM,
            TYPE_DIGITAL,
            TYPE_ANALOG
        };

        typedef std::function<int(void)> ValueGeneratorFunction;

        // add units?
        struct Sensor {
            int pin;
            const char * name;
            SensorType type;
            ValueGeneratorFunction valueGenerator = [](){return -1;};

            Sensor * next;
            Sensor * previous;
        };

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
            private:
                Sensor * _head;
                int _count;


                bool add(const char * name, SensorType type, ValueGeneratorFunction valueGenerator);
                void append(Sensor * sensor);
                const Sensor * findSensor(const char * name) const;
        };
    }
}

#endif