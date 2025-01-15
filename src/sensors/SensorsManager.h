#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <list>

#include "Features.h"
#include "sensors/Sensor.h"
#include "logs/BetterLogger.h"

#if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS

const char * const _SENSORS_MANAGER_TAG = "sensors-manager";

enum SensorType {
  UNKNOWN_SENSOR,
  NUMBER_SENSOR,
  TEXT_SENSOR
};

inline const char * sensorTypeToStr(SensorType type) {
  switch (type) {
    case NUMBER_SENSOR:
      return "number";
    case TEXT_SENSOR:
      return "text";
    default:
      return "unknown";
  }
}

class SensorsManagerClass {
  public:
    #if ENABLE_NUMBER_SENSORS
      /*
        Add number sensor
        @param name unique sensor system name
        @param valueProvider lambda with logic for calculating sensor value
        @return true if sensor added
      */
      bool addSensor(const char * name, typename Sensor<NUMBER_SENSOR_DATA_TYPE>::ValueProvider valueProvider) {
        return addSensor<NUMBER_SENSOR_DATA_TYPE>(name, valueProvider);
      }

      /*
        Add digital sensor (uses digitalRead)
        @param name unique sensor system name
        @param pin digital sensor pin
        @param mode pin mode
        @returns true if sensor added
      */
      bool addDigitalSensor(const char* name, uint8_t pin, uint8_t mode = INPUT_PULLUP);
      /*
        Add analog sensor (uses analogRead)
        @param name unique sensor system name
        @param pin analog sensor pin
        @returns true if sensor added
      */
      bool addAnalogSensor(const char* name, uint8_t pin);
    #endif
    #if ENABLE_TEXT_SENSORS
      /*
        Add text sensor
        @param name unique unique sensor system name
        @param valueProvider lambda with logic for calculating sensor value
        @return true if sensor added
      */
      bool addSensor(const char * name, typename Sensor<TEXT_SENSOR_DATA_TYPE>::ValueProvider valueProvider) {
        return addSensor<TEXT_SENSOR_DATA_TYPE>(name, valueProvider);
      }
    #endif

    size_t count();
    JsonDocument getSensorsInfo();

    template<typename T>
    const Sensor<T> * getSensor(const char * name) {
      auto it = getSensorIterator<T>(name);
      if (it == getList<T>()->end()) {
        return nullptr;
      }
      return *it;
    }
    
    SensorType getSensorType(const char * name) {
      #if ENABLE_NUMBER_SENSORS
        if (getSensorIterator<NUMBER_SENSOR_DATA_TYPE>(name) != _sensorsList.end()) {
          return NUMBER_SENSOR;
        }
      #endif
      #if ENABLE_TEXT_SENSORS
        if (getSensorIterator<TEXT_SENSOR_DATA_TYPE>(name) != _deviceStatesList.end()) {
          return TEXT_SENSOR;
        }
      #endif
      return UNKNOWN_SENSOR;
    }
  private:
    #if ENABLE_NUMBER_SENSORS
      std::list<Sensor<NUMBER_SENSOR_DATA_TYPE>*> _sensorsList;
    #endif

    #if ENABLE_TEXT_SENSORS
      std::list<Sensor<TEXT_SENSOR_DATA_TYPE>*> _deviceStatesList;
    #endif

    template<typename T>
    bool addSensor(
      const char* name,
      typename Sensor<T>::ValueProvider valueProvider
    )  {
      bool exists = false;
      #if ENABLE_NUMBER_SENSORS
        exists = getSensorIterator<NUMBER_SENSOR_DATA_TYPE>(name) != _sensorsList.end();
      #endif

      #if ENABLE_TEXT_SENSORS
        exists = exists || getSensorIterator<TEXT_SENSOR_DATA_TYPE>(name) != _deviceStatesList.end();
      #endif

      if (exists) {
        st_log_warning(_SENSORS_MANAGER_TAG, "Sensor with name %s already exist! Skipping...", name);
        return false;
      }

      getList<T>()->push_back(new Sensor<T>(name, valueProvider));
      st_log_debug(_SENSORS_MANAGER_TAG, "Added new device sensor %s", name);
      return true;
    }

    template<typename T>
    std::list<Sensor<T>*> * getList();

    template<typename T>
    typename std::list<Sensor<T>*>::iterator getSensorIterator(const char * name) {
      auto list = getList<T>();
      if (name == nullptr || strlen(name) == 0) {
        return list->end();
      }

      return std::find_if(list->begin(), list->end(), [name](const Sensor<T> * sensor) {
          return strcmp(sensor->name(), name) == 0;
      });
    }

  };

extern SensorsManagerClass SensorsManager;

#endif

#endif