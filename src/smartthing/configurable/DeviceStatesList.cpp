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
        doc[currentDeviceState->name] = currentDeviceState->valueGenerator();
        currentDeviceState = currentDeviceState->next;
    }
    return doc;
}

bool DeviceStatesList::add(const char * name, ValueGeneratorFunction function) {
    if (findState(name) != nullptr) {
        LOGGER.warning(DEVICE_STATES_LIST_TAG, "State with name %s already exist! Skipping...", name);
        return false;
    }
    DeviceState * sensor = new DeviceState();
    sensor->name = name;
    sensor->valueGenerator = function;
    append(sensor);
    LOGGER.debug(DEVICE_STATES_LIST_TAG, "Added new device state %s", name);
    return true;
}

void DeviceStatesList::append(DeviceState * sensor) {
    sensor->next = _head;
    if (_head != nullptr) {
        _head->previous = sensor;
    }
    _head = sensor;
    _count++;
}

const DeviceState * DeviceStatesList::findState(const char * name) const {
    DeviceState * current = _head;
    while (current != nullptr) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}
