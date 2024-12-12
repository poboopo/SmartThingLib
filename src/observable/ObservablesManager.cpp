#include "observable/ObservablesManager.h"
#include "logs/BetterLogger.h"

ObservablesManagerClass ObservablesManager;

#if ENABLE_SENSORS
template<>
List<ObservableObject<NUMBER_SENSOR_TYPE>> * ObservablesManagerClass::getList() {
  return &_sensorsList;
}
#endif

#if ENABLE_STATES
template<>
List<ObservableObject<TEXT_SENSOR_TYPE>> * ObservablesManagerClass::getList() {
  return &_deviceStatesList;
}
#endif

#if ENABLE_SENSORS

bool ObservablesManagerClass::addDigitalSensor(const char* name, uint8_t pin, uint8_t mode) {
  pinMode(pin, mode);
  return addSensor<NUMBER_SENSOR_TYPE>(name, [pin]() {
    if (pin > 0) {
      return digitalRead(pin);
    }
    return -1;
  });
}

bool ObservablesManagerClass::addAnalogSensor(const char* name, uint8_t pin) {
  return addSensor<NUMBER_SENSOR_TYPE>(name, [pin]() {
    if (pin > 0) {
      return (int)analogRead(pin);
    }
    return -1;
  });
}

#endif

size_t ObservablesManagerClass::getSensorsCount() {
  size_t result = 0;
  
  #ifdef ENABLE_SENSORS
    result += getList<NUMBER_SENSOR_TYPE>()->size();
  #endif
  
  #ifdef ENABLE_STATES
    result += getList<TEXT_SENSOR_TYPE>()->size();
  #endif

  return result;
}

JsonDocument ObservablesManagerClass::getObservablesInfo(bool full) {
  JsonDocument doc;
  
  if (full) {
    doc.to<JsonArray>();
    JsonArray array = doc.as<JsonArray>();

    #if ENABLE_SENSORS
      collectObservablesInfoFull<NUMBER_SENSOR_TYPE>(&_sensorsList, array, OBS_SENSOR);
    #endif

    #if ENABLE_STATES
      collectObservablesInfoFull<TEXT_SENSOR_TYPE>(&_deviceStatesList, array, OBS_STATE);
    #endif
  } else {
    doc.to<JsonObject>();
    JsonObject object = doc.as<JsonObject>();

    #if ENABLE_SENSORS
      _sensorsList.forEach([&](ObservableObject<NUMBER_SENSOR_TYPE> * obj) {
        object[obj->name()] = obj->provideValue();
      });
    #endif

    #if ENABLE_STATES
    _deviceStatesList.forEach([&](ObservableObject<TEXT_SENSOR_TYPE> * obj) {
      object[obj->name()] = obj->provideValue();
    });
    #endif
  }

  return doc;
}