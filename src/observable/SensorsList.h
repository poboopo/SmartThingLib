#ifndef SENSORS_LIST_H
#define SENSORS_LIST_H

#include "Features.h"
#if ENABLE_SENSORS 

#include <Arduino.h>
#include <ArduinoJson.h>

#include "logs/BetterLogger.h"
#include "observable/ObservableObjects.h"
#include "utils/List.h"

static const char * SENSORS_LIST_TAG = "sensors_list";

namespace Observable {
namespace Sensor {
class SensorsList : public List<Sensor> {
 public:
  bool add(const char* name,
           Observable::ObservableObject<int16_t>::ValueProviderFunction
               valueProvider) {
    return add(name, TYPE_CUSTOM, valueProvider);
  };
  bool addDigital(const char* name, int pin) {
    if (pin < 0) {
      ST_LOG_WARNING(SENSORS_LIST_TAG,
                     "Pin value can't be negative for sensor! Skipping...");
      return false;
    }
    return add(name, TYPE_DIGITAL, [pin]() {
      if (pin > 0) {
        return digitalRead(pin);
      }
      return -1;
    });
  };
  bool addAnalog(const char* name, int pin) {
    if (pin < 0) {
      ST_LOG_WARNING(SENSORS_LIST_TAG,
                     "Pin value can't be negative for sensor! Skipping...");
      return false;
    }
    return add(name, TYPE_ANALOG, [pin]() {
      if (pin > 0) {
        return (int)analogRead(pin);
      }
      return -1;
    });
  };

  JsonDocument getValues() {
    JsonDocument doc;
    forEach([&](Sensor* current) {
      doc[current->name] = current->valueProvider();
    });
    return doc;
  };
  JsonDocument getTypes() {
    JsonDocument doc;
    forEach([&](Sensor* current) {
      doc[current->name] = Observable::Sensor::sensorTypeName(current->type);
    });
    return doc;
  }

  const Sensor* findSensor(const char* name) {
    return findValue(
        [&](Sensor* current) { return strcmp(current->name, name) == 0; });
  };

 private:
  bool add(const char* name, SensorType type,
           Observable::ObservableObject<int16_t>::ValueProviderFunction
               valueProvider) {
    if (findSensor(name) != nullptr) {
      ST_LOG_WARNING(SENSORS_LIST_TAG,
                     "Sensor with name %s already exist! Skipping...", name);
      return false;
    }
    Sensor* sensor = new Sensor();
    sensor->name = name;
    sensor->valueProvider = valueProvider;
    sensor->type = type;
    if (append(sensor) > -1) {
      ST_LOG_DEBUG(SENSORS_LIST_TAG, "Added new sensor %s", name);
      return true;
    } else {
      if (sensor != nullptr) {
        delete sensor;
      }
      ST_LOG_ERROR(SENSORS_LIST_TAG, "Failed to add new sensor %s", name);
      return false;
    }
  };
};
}  // namespace Sensor
}  // namespace Observable

#endif

#endif