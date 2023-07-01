#include "smartthing/configurable/DeviceStatesList.h"
#include "smartthing/logs/BetterLogger.h"

#define DEVICE_STATES_LIST_TAG "device_states_list"

using namespace Configurable::DeviceState;

DeviceStatesList::~DeviceStatesList() {
    DeviceState * current = _head;
    while (current->next != nullptr) {
        current = current->next;
        delete(current->previous);
    }
    delete(current);
}

DynamicJsonDocument DeviceStatesList::getValues() {
    DynamicJsonDocument doc(_count * 64);
    DeviceState * currentDeviceState = _head;
    while (currentDeviceState != nullptr) {
        JsonObject sensorObj = doc.createNestedObject();
        sensorObj["name"] = currentDeviceState->name;
        sensorObj["value"] = currentDeviceState->valueGenerator();
        currentDeviceState = currentDeviceState->next;
    }
    return doc;
}

void DeviceStatesList::add(const char * name, ValueGeneratorFunction function) {
    DeviceState * sensor = new DeviceState();
    sensor->name = name;
    sensor->valueGenerator = function;
    append(sensor);
    LOGGER.debug(DEVICE_STATES_LIST_TAG, "Added new device state %s", name);
}

void DeviceStatesList::append(DeviceState * sensor) {
    sensor->next = _head;
    if (_head != nullptr) {
        _head->previous = sensor;
    }
    _head = sensor;
    _count++;
}