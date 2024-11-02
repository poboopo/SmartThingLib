#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "Features.h"

#include <ArduinoJson.h>
#include <functional>

#define STATE_TYPE "state"
#define SENSOR_TYPE "sensor"

template <class T>
struct ObservableObject {
public:
  ObservableObject(const char* objType) : type(objType){};
  typedef std::function<T(void)> ValueProviderFunction;

  const char* name;
  const char* type;

  ValueProviderFunction valueProvider;

  JsonDocument toJson() {
    JsonDocument doc;
    doc["name"] = name;
    doc["type"] = type;
    return doc;
  };
};

#if ENABLE_SENSORS
enum SensorType { TYPE_CUSTOM, TYPE_DIGITAL, TYPE_ANALOG };
inline const char* sensorTypeName(SensorType type) {
  switch (type) {
    case TYPE_CUSTOM:
      return "custom";
    case TYPE_DIGITAL:
      return "digital";
    case TYPE_ANALOG:
      return "analog";
  }
  return "type_not_found_how";
};

struct Sensor : public ObservableObject<int16_t> {
public:
  Sensor() : ObservableObject<int16_t>(SENSOR_TYPE){};
  int pin;
  SensorType type;
};
#endif

#if ENABLE_STATES
struct DeviceState : public ObservableObject<String> {
  DeviceState() : ObservableObject<String>(STATE_TYPE){};
};
#endif

#endif