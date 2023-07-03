#ifndef DEVICE_STATES_LIST_H
#define DEVICE_STATES_LIST_H

#include <functional>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "smartthing/configurable/ConfigurableObjects.h"

namespace Configurable {
    namespace DeviceState {
        class DeviceStatesList {
            public:
                DeviceStatesList(): _head(nullptr){};
                ~DeviceStatesList();
                bool add(const char * name, ValueGeneratorFunction valueGenerator);
                DynamicJsonDocument getValues();
                int size() {
                    return _count;
                }
                const DeviceState * findState(const char * name) const;
            private:
                DeviceState * _head;
                int _count;
                void append(DeviceState * sensor);
        };
    }
}

#endif