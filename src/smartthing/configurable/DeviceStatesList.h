#ifndef DEVICE_STATES_LIST_H
#define DEVICE_STATES_LIST_H

#include <functional>
#include <ArduinoJson.h>
#include <Arduino.h>

namespace Configurable {
    namespace DeviceState {
        typedef std::function<const char *(void)> ValueGeneratorFunction;

        // add units?
        struct DeviceState {
            const char * name;
            ValueGeneratorFunction valueGenerator = [](){return "";};

            DeviceState * next;
            DeviceState * previous;
        };

        class DeviceStatesList {
            public:
                DeviceStatesList(): _head(nullptr){};
                ~DeviceStatesList();
                void add(const char * name, ValueGeneratorFunction valueGenerator);
                DynamicJsonDocument getValues();
                int size() {
                    return _count;
                }
            private:
                DeviceState * _head;
                int _count;
                void append(DeviceState * sensor);
        };
    }
}

#endif