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
        JsonObject sensorObj = doc.createNestedObject();
        sensorObj["name"] = currentSensor->name;
        sensorObj["value"] = currentSensor->valueGenerator();
        currentSensor = currentSensor->next;
    }
    return doc;
}

void SensorsList::add(const char * name, ValueGeneratorFunction function) {
    Sensor * sensor = new Sensor();
    sensor->name = name;
    sensor->valueGenerator = function;
    append(sensor);
    LOGGER.debug(SENSORS_LIST_TAG, "Added new sensor %s", name);
}

void SensorsList::addDigital(const char * name, int pin) {
    Sensor * sensor = new Sensor();
    sensor->name = name;
    sensor->pin = pin;
    sensor->valueGenerator = [pin]() {
        if (pin > 0) {
            return digitalRead(pin);
        }
        return -1;
    };
    append(sensor);
    LOGGER.debug(SENSORS_LIST_TAG, "Added new digital sensor %s on pin %d", name, pin);
}

void SensorsList::addAnalog(const char * name, int pin) {
    Sensor * sensor = new Sensor();
    sensor->name = name;
    sensor->pin = pin;
    sensor->valueGenerator = [pin]() {
        if (pin > 0) {
            return (int) analogRead(pin);
        }
        return -1;
    };
    append(sensor);
    LOGGER.debug(SENSORS_LIST_TAG, "Added new analog sensor %s on pin %d", name, pin);
}

void SensorsList::append(Sensor * sensor) {
    sensor->next = _head;
    if (_head != nullptr) {
        _head->previous = sensor;
    }
    _head = sensor;
    _count++;
}