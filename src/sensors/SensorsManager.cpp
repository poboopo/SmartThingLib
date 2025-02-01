#include "Features.h"

#if ENABLE_ENABLE_TEXT_SENSORS || ENABLE_NUMBER_SENSORS

#include "sensors/SensorsManager.h"
#include "logs/BetterLogger.h"

SensorsManagerClass SensorsManager;

#if ENABLE_TEXT_SENSORS
template<>
std::list<Sensor<TEXT_SENSOR_DATA_TYPE>*> * SensorsManagerClass::getList() {
  return &_deviceStatesList;
}
#endif

#if ENABLE_NUMBER_SENSORS
template<>
std::list<Sensor<NUMBER_SENSOR_DATA_TYPE>*> * SensorsManagerClass::getList() {
  return &_sensorsList;
}

bool SensorsManagerClass::addDigital(const char* name, uint8_t pin, uint8_t mode) {
  pinMode(pin, mode);
  return add<NUMBER_SENSOR_DATA_TYPE>(name, [pin]() {
    if (pin > 0) {
      return digitalRead(pin);
    }
    return -1;
  });
}

bool SensorsManagerClass::addAnalog(const char* name, uint8_t pin) {
  return add<NUMBER_SENSOR_DATA_TYPE>(name, [pin]() {
    if (pin > 0) {
      return (int)analogRead(pin);
    }
    return -1;
  });
}

#endif

size_t SensorsManagerClass::count() {
  size_t result = 0;
  
  #if ENABLE_NUMBER_SENSORS
    result += getList<NUMBER_SENSOR_DATA_TYPE>()->size();
  #endif
  
  #if ENABLE_TEXT_SENSORS
    result += getList<TEXT_SENSOR_DATA_TYPE>()->size();
  #endif

  return result;
}

// todo remove arduino json
JsonDocument SensorsManagerClass::getSensorsInfo() {
  JsonDocument doc;
  
  doc.to<JsonObject>();
  JsonObject object = doc.as<JsonObject>();

  #if ENABLE_NUMBER_SENSORS
    for (auto it = _sensorsList.begin(); it != _sensorsList.end(); ++it) {
      object[(*it)->name()] = (*it)->provideValue();
    }
  #endif

  #if ENABLE_TEXT_SENSORS
    for (auto it = _deviceStatesList.begin(); it != _deviceStatesList.end(); ++it) {
      object[(*it)->name()] = (*it)->provideValue();
    }
  #endif

  return doc;
}

#endif