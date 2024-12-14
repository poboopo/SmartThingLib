#ifndef SENSORS_MANAGER_H
#define SENSORS_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Features.h"
#include "sensors/Sensor.h"
#include "utils/List.h"
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
      bool addSensor(const char * name, typename Sensor<NUMBER_SENSOR_TYPE>::ValueProvider valueProvider) {
        return addSensor<NUMBER_SENSOR_TYPE>(name, valueProvider);
      }
      bool addDigitalSensor(const char* name, uint8_t pin, uint8_t mode = INPUT_PULLUP);
      bool addAnalogSensor(const char* name, uint8_t pin);
    #endif
    #if ENABLE_TEXT_SENSORS
      bool addSensor(const char * name, typename Sensor<TEXT_SENSOR_TYPE>::ValueProvider valueProvider) {
        return addSensor<TEXT_SENSOR_TYPE>(name, valueProvider);
      }
    #endif

    size_t getSensorsCount();
    JsonDocument getSensorsInfo();

    template<typename T>
    bool addSensor(
      const char* name,
      typename Sensor<T>::ValueProvider valueProvider
    )  {
      bool exists = false;
      #if ENABLE_NUMBER_SENSORS
        exists = getSensor<NUMBER_SENSOR_TYPE>(name) != nullptr;
      #endif

      #if ENABLE_TEXT_SENSORS
        exists = exists || getSensor<TEXT_SENSOR_TYPE>(name) != nullptr;
      #endif

      if (exists) {
        st_log_warning(_SENSORS_MANAGER_TAG, "Sensor with name %s already exist! Skipping...", name);
        return false;
      }

      Sensor<T> * sensor = new Sensor<T>(name, valueProvider);
      if (getList<T>()->append(sensor) > -1) {
        st_log_debug(_SENSORS_MANAGER_TAG, "Added new device state %s", name);
        return true;
      } else {
        if (sensor != nullptr) {
          delete sensor;
        }
        st_log_error(_SENSORS_MANAGER_TAG, "Dailed to add new device state %s", name);
        return false;
      }
    }

    template<typename T>
    const Sensor<T> * getSensor(const char * name) {
      if (name == nullptr || strlen(name) == 0) {
        return nullptr;
      }

      return getList<T>()->findValue([&](Sensor<T> * current) {
          return strcmp(current->name(), name) == 0;
      });
    }
    SensorType getSensorType(const char * name) {
      #if ENABLE_NUMBER_SENSORS
      if (getSensor<NUMBER_SENSOR_TYPE>(name) != nullptr) {
        return NUMBER_SENSOR;
      }
      #endif
      #if ENABLE_TEXT_SENSORS
      if (getSensor<TEXT_SENSOR_TYPE>(name) != nullptr) {
        return TEXT_SENSOR;
      }
      #endif
      return UNKNOWN_SENSOR;
    }
  private:
    #if ENABLE_NUMBER_SENSORS
      List<Sensor<NUMBER_SENSOR_TYPE>> _sensorsList;
    #endif

    #if ENABLE_TEXT_SENSORS
      List<Sensor<TEXT_SENSOR_TYPE>> _deviceStatesList;
    #endif

    template<typename T>
    List<Sensor<T>> * getList();
  };

extern SensorsManagerClass SensorsManager;

#endif

#endif