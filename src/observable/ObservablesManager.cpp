#include "observable/ObservablesManager.h"

ObservablesManagerClass ObservablesManager;

#if ENABLE_SENSORS 
JsonDocument ObservablesManagerClass::getSensorsValues() {
  return _sensorsList.getValues();
} 
JsonDocument ObservablesManagerClass::getSensorsTypes() {
  return _sensorsList.getTypes();
}

int16_t ObservablesManagerClass::getSensorsCount() { return _sensorsList.size(); }

bool ObservablesManagerClass::addSensor(
    const char* name,
    ObservableObject<int16_t>::ValueProviderFunction function) {
  return _sensorsList.add(name, function);
}

bool ObservablesManagerClass::addDigitalSensor(const char* name, int pin) {
  pinMode(pin, INPUT_PULLUP);
  return _sensorsList.addDigital(name, pin);
}

bool ObservablesManagerClass::addAnalogSensor(const char* name, int pin) {
  return _sensorsList.addAnalog(name, pin);
}

const Sensor* ObservablesManagerClass::getSensor(
    const char* name) {
  return _sensorsList.findSensor(name);
}
#endif

#if ENABLE_STATES
bool ObservablesManagerClass::addDeviceState(
    const char* name,
    ObservableObject<const char*>::ValueProviderFunction
        function) {
  return _deviceStatesList.add(name, function);
}

JsonDocument ObservablesManagerClass::getDeviceStatesInfo() {
  return _deviceStatesList.getValues();
}

const DeviceState* ObservablesManagerClass::getDeviceState(
    const char* name) {
  return _deviceStatesList.findState(name);
}

int16_t ObservablesManagerClass::getDeviceStatesCount() {
  return _deviceStatesList.size();
}

#endif