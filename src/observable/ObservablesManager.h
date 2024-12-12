#ifndef OBSERVABLES_MANAGER_H
#define OBSERVABLES_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Features.h"
#include "observable/ObservableObject.h"
#include "utils/List.h"
#include "logs/BetterLogger.h"

#if ENABLE_SENSORS || ENABLE_STATES

const char * const _OBSERVABLES_MANAGER_TAG = "observables-manager";

class ObservablesManagerClass {
  public:
    #if ENABLE_SENSORS
      bool addSensor(const char * name, typename ObservableObject<NUMBER_SENSOR_TYPE>::ValueProvider valueProvider) {
        return addSensor<NUMBER_SENSOR_TYPE>(name, valueProvider);
      }
      bool addDigitalSensor(const char* name, uint8_t pin, uint8_t mode = INPUT_PULLUP);
      bool addAnalogSensor(const char* name, uint8_t pin);
    #endif
    #if ENABLE_STATES
      bool addSensor(const char * name, typename ObservableObject<TEXT_SENSOR_TYPE>::ValueProvider valueProvider) {
        return addSensor<TEXT_SENSOR_TYPE>(name, valueProvider);
      }
    #endif

    size_t getSensorsCount();
    JsonDocument getObservablesInfo(bool full = false);

    template<typename T>
    bool addSensor(
      const char* name,
      typename ObservableObject<T>::ValueProvider valueProvider
    )  {
      bool exists = false;
      #if ENABLE_SENSORS
        exists = getObservableObject<NUMBER_SENSOR_TYPE>(name) != nullptr;
      #endif

      #if ENABLE_STATES
        exists = exists || getObservableObject<TEXT_SENSOR_TYPE>(name) != nullptr;
      #endif

      if (exists) {
        st_log_warning(_OBSERVABLES_MANAGER_TAG, "Sensor with name %s already exist! Skipping...", name);
        return false;
      }

      ObservableObject<T> * sensor = new ObservableObject<T>(name, valueProvider);
      if (getList<T>()->append(sensor) > -1) {
        st_log_debug(_OBSERVABLES_MANAGER_TAG, "Added new device state %s", name);
        return true;
      } else {
        if (sensor != nullptr) {
          delete sensor;
        }
        st_log_error(_OBSERVABLES_MANAGER_TAG, "Dailed to add new device state %s", name);
        return false;
      }
    }

    template<typename T>
    const ObservableObject<T> * getObservableObject(const char * name) {
      return getList<T>()->findValue([&](ObservableObject<T> * current) {
          return strcmp(current->name(), name) == 0;
      });
    }
  private:
    #if ENABLE_SENSORS
      List<ObservableObject<NUMBER_SENSOR_TYPE>> _sensorsList;
    #endif

    #if ENABLE_STATES
      List<ObservableObject<TEXT_SENSOR_TYPE>> _deviceStatesList;
    #endif

    template<typename T>
    List<ObservableObject<T>> * getList();

    template<typename T>
    void collectObservablesInfoFull(List<ObservableObject<T>> * values, JsonArray &array, ObservableType type) {
      const char * typeStr = observableTypeToStr(type);

      values->forEach([&](ObservableObject<T> * obj) {
        JsonDocument doc;
        doc["name"] = obj->name();
        doc["value"] = obj->provideValue();
        doc["type"] = typeStr;

        array.add(doc);
      });
    }
};

extern ObservablesManagerClass ObservablesManager;

#endif

#endif