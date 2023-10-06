#ifndef DEVICE_STATES_LIST_H
#define DEVICE_STATES_LIST_H

#include <functional>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "configurable/ConfigurableObjects.h"
#include "utils/List.h"

#define DEVICE_STATES_LIST_TAG "device_states_list"

namespace Configurable {
    namespace DeviceState {
        class DeviceStatesList: public List<DeviceState> {
            public:
                bool add(const char * name, Configurable::ConfigurableObject<const char *>::ValueGeneratorFunction valueGenerator) {
                    if (findState(name) != nullptr) {
                        LOGGER.warning(DEVICE_STATES_LIST_TAG, "State with name %s already exist! Skipping...", name);
                        return false;
                    }
                    DeviceState * state = new DeviceState();
                    state->name = name;
                    state->valueGenerator = valueGenerator;
                    if (append(state) > -1) {
                        LOGGER.debug(DEVICE_STATES_LIST_TAG, "Added new device state %s", name);
                        return true;
                    } else {
                        if (state != nullptr) {
                            delete(state);
                        }
                        LOGGER.error(DEVICE_STATES_LIST_TAG, "Dailed to add new device state %s", name);
                        return false;
                    }
                };
                DynamicJsonDocument getValues() {
                    DynamicJsonDocument doc(size() * 64);
                    forEach([&](DeviceState * current) {
                        doc[current->name] = current->valueGenerator();
                    });
                    return doc;
                };
                const DeviceState * findState(const char * name) {
                    return findValue([&](DeviceState * current) {
                        return strcmp(current->name, name) == 0;
                    });
                };
        };
    }
}

#endif