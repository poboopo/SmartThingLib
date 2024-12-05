#ifndef OBSERVABLES_MANAGER_H
#define OBSERVABLES_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Features.h"
#include "observable/ObservableObjects.h"
#include "utils/List.h"

#if ENABLE_SENSORS || ENABLE_STATES

class ObservablesManagerClass {
  public:
    #if ENABLE_SENSORS
      bool addDigitalSensor(const char* name, uint8_t pin, uint8_t mode = INPUT_PULLUP);
      
      bool addAnalogSensor(const char* name, uint8_t pin);

      bool addSensorCustom(
        const char* name,
        ObservableObject<SENSOR_DATA_TYPE>::ValueProviderFunction valueProvider
      );

      uint8_t getSensorsCount();
    #endif

    #if ENABLE_STATES
      bool addDeviceState(const char* name, ObservableObject<STATE_DATA_TYPE>::ValueProviderFunction valueProvider);

      uint8_t getDeviceStatesCount();
    #endif

    template<typename T>
    const ObservableObject<T> * getObservableObject(const char * name);

    JsonDocument getObservablesInfo(bool full = false);
  private:
    #if ENABLE_SENSORS
      List<ObservableObject<SENSOR_DATA_TYPE>> _sensorsList;
      bool addSensor(const char* name, SensorType type, ObservableObject<SENSOR_DATA_TYPE>::ValueProviderFunction valueProvider);
    #endif

    #if ENABLE_STATES
      List<ObservableObject<STATE_DATA_TYPE>> _deviceStatesList;
    #endif

    template<typename T>
    void collectObservablesInfoFull(List<ObservableObject<T>> * values, JsonArray &array, ObservableType type);
};

extern ObservablesManagerClass ObservablesManager;

#endif

#endif