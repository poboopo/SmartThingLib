#ifndef SENSORS_LIST_H
#define SENSORS_LIST_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include "configurable/ConfigurableObjects.h"
#include "utils/List.h"

#define SENSORS_LIST_TAG "sensors_list"

namespace Configurable {
    namespace Sensor {
        class SensorsList: public List<Sensor> {
            public:
                bool add(const char * name, Configurable::ConfigurableObject<int16_t>::ValueGeneratorFunction valueGenerator) {
                    return add(name, TYPE_CUSTOM, valueGenerator);
                };
                bool addDigital(const char * name, int pin) {
                    if (pin < 0) {
                        LOGGER.warning(SENSORS_LIST_TAG, "Pin value can't be negative for sensor! Skipping...");
                        return false;
                    }
                    return add(name, TYPE_DIGITAL, [pin]() {
                        if (pin > 0) {
                            return digitalRead(pin);
                        }
                        return -1;
                    });
                };
                bool addAnalog(const char * name, int pin) {
                    if (pin < 0) {
                        LOGGER.warning(SENSORS_LIST_TAG, "Pin value can't be negative for sensor! Skipping...");
                        return false;
                    }
                    return add(name, TYPE_ANALOG, [pin]() {
                        if (pin > 0) {
                            return (int) analogRead(pin);
                        }
                        return -1;
                    });
                };

                DynamicJsonDocument getValues() {
                    DynamicJsonDocument doc(size() * 64);
                    forEach([&](Sensor * current) {
                        JsonObject sensorObj = doc.createNestedObject(current->name);
                        sensorObj["value"] = current->valueGenerator();
                        sensorObj["type"] = Configurable::Sensor::sensorTypeName(current->type);
                    });
                    return doc;
                };
                const Sensor * findSensor(const char * name) {
                    return findValue([&](Sensor * current) {
                        return strcmp(current->name, name) == 0;
                    });
                };
            private:
                bool add(const char * name, SensorType type, Configurable::ConfigurableObject<int16_t>::ValueGeneratorFunction valueGenerator) {
                    if (findSensor(name) != nullptr) {
                        LOGGER.warning(SENSORS_LIST_TAG, "Sensor with name %s already exist! Skipping...", name);
                        return false;
                    }
                    Sensor * sensor = new Sensor();
                    sensor->name = name;
                    sensor->valueGenerator = valueGenerator;
                    sensor->type = type;
                    if (append(sensor) > -1) {
                        LOGGER.debug(SENSORS_LIST_TAG, "Added new sensor %s", name);
                        return true;
                    } else {
                        if (sensor != nullptr) {
                            delete(sensor);
                        }
                        LOGGER.error(SENSORS_LIST_TAG, "Failed to add new sensor %s", name);
                        return false;
                    }
                };
        };
    }
}

#endif