#include "smartthing/configurable/SensorsList.h"
#include "smartthing/logs/BetterLogger.h"

#define SENSORS_LIST_TAG "sensors_list"

using namespace Configurable::Sensor;

SensorsList::~SensorsList() {
    Sensor * current = _head;
    while (current->next != nullptr) {
        current = current->next;
        delete(current->previous);
    }
    delete(current);
}

DynamicJsonDocument SensorsList::getValues() {
    DynamicJsonDocument doc(_count * 64);
    Sensor * currentSensor = _head;
    while (currentSensor != nullptr) {
        JsonObject sensorObj = doc.createNestedObject(currentSensor->name);
        sensorObj["value"] = currentSensor->valueGenerator();
        sensorObj["type"] = Configurable::Sensor::sensorTypeName(currentSensor->type);
        currentSensor = currentSensor->next;
    }
    return doc;
}

bool SensorsList::add(const char * name, SensorType type, ValueGeneratorFunction function) {
    if (findSensor(name) != nullptr) {
        LOGGER.warning(SENSORS_LIST_TAG, "Sensor with name %s already exist! Skipping...", name);
        return false;
    }
    Sensor * sensor = new Sensor();
    sensor->name = name;
    sensor->valueGenerator = function;
    sensor->type = type;
    append(sensor);
    LOGGER.debug(SENSORS_LIST_TAG, "Added new sensor %s", name);
    return true;
}

bool SensorsList::add(const char * name, ValueGeneratorFunction function) {
    return add(name, TYPE_CUSTOM, function);
}

bool SensorsList::addDigital(const char * name, int pin) {
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
}

bool SensorsList::addAnalog(const char * name, int pin) {
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
}

void SensorsList::append(Sensor * sensor) {
    sensor->next = _head;
    if (_head != nullptr) {
        _head->previous = sensor;
    }
    _head = sensor;
    _count++;
}

const Sensor * SensorsList::findSensor(const char * name) const {
    Sensor * current = _head;
    while (current != nullptr) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}