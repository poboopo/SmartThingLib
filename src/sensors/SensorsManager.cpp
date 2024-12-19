#include "sensors/SensorsManager.h"
#include "logs/BetterLogger.h"

SensorsManagerClass SensorsManager;

#if ENABLE_NUMBER_SENSORS
template<>
List<Sensor<TEXT_SENSOR_DATA_TYPE>> * SensorsManagerClass::getList() {
  return &_deviceStatesList;
}

template<>
List<Sensor<NUMBER_SENSOR_DATA_TYPE>> * SensorsManagerClass::getList() {
  return &_sensorsList;
}

bool SensorsManagerClass::addDigitalSensor(const char* name, uint8_t pin, uint8_t mode) {
  pinMode(pin, mode);
  return addSensor<NUMBER_SENSOR_DATA_TYPE>(name, [pin]() {
    if (pin > 0) {
      return digitalRead(pin);
    }
    return -1;
  });
}

bool SensorsManagerClass::addAnalogSensor(const char* name, uint8_t pin) {
  return addSensor<NUMBER_SENSOR_DATA_TYPE>(name, [pin]() {
    if (pin > 0) {
      return (int)analogRead(pin);
    }
    return -1;
  });
}

#endif

size_t SensorsManagerClass::getSensorsCount() {
  size_t result = 0;
  
  #if ENABLE_NUMBER_SENSORS
    result += getList<NUMBER_SENSOR_DATA_TYPE>()->size();
  #endif
  
  #if ENABLE_TEXT_SENSORS
    result += getList<TEXT_SENSOR_DATA_TYPE>()->size();
  #endif

  return result;
}

JsonDocument SensorsManagerClass::getSensorsInfo() {
  JsonDocument doc;
  
  doc.to<JsonObject>();
  JsonObject object = doc.as<JsonObject>();

  #if ENABLE_NUMBER_SENSORS
    _sensorsList.forEach([&](Sensor<NUMBER_SENSOR_DATA_TYPE> * obj) {
      object[obj->name()] = obj->provideValue();
    });
  #endif

  #if ENABLE_TEXT_SENSORS
  _deviceStatesList.forEach([&](Sensor<TEXT_SENSOR_DATA_TYPE> * obj) {
    object[obj->name()] = obj->provideValue();
  });
  #endif

  return doc;
}