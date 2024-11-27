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
    ObservableObject<SENSOR_DATA_TYPE>::ValueProviderFunction function) {
  return _sensorsList.add(name, function);
}

bool ObservablesManagerClass::addDigitalSensor(const char* name, int pin) {
  pinMode(pin, INPUT_PULLUP);
  return _sensorsList.addDigital(name, pin);
}

bool ObservablesManagerClass::addAnalogSensor(const char* name, int pin) {
  return _sensorsList.addAnalog(name, pin);
}

template<>
const ObservableObject<SENSOR_DATA_TYPE> * ObservablesManagerClass::getObservableObject(const char * name) {
  return _sensorsList.findSensor(name);
}

const Sensor* ObservablesManagerClass::getSensor(const char* name) {
  const ObservableObject<SENSOR_DATA_TYPE> * obj = getObservableObject<SENSOR_DATA_TYPE>(name);
  return obj == nullptr ? nullptr : (const Sensor *) obj;
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

template<>
const ObservableObject<STATE_DATA_TYPE> * ObservablesManagerClass::getObservableObject(const char * name) {
  return _deviceStatesList.findState(name);
}

const DeviceState* ObservablesManagerClass::getDeviceState(const char* name) {
  const ObservableObject<STATE_DATA_TYPE> * obj = getObservableObject<STATE_DATA_TYPE>(name);
  return obj == nullptr ? nullptr : (const DeviceState *) obj;
}

int16_t ObservablesManagerClass::getDeviceStatesCount() {
  return _deviceStatesList.size();
}

#endif