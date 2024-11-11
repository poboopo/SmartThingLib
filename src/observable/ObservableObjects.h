#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "Features.h"

#include <ArduinoJson.h>
#include <functional>

static const char * _state = "state";
static const char * _sensor = "sensor";

enum ObservableType {
  UNKNOWN_OBS_TYPE,
  OBS_STATE,
  OBS_SENSOR
};

inline const char * observableTypeToStr(ObservableType type) {
  switch (type) {
    case OBS_STATE:
      return _state;
    case OBS_SENSOR:
      return _sensor;
    default:
      return "unknown";
  }
}

inline ObservableType observableTypeFromStr(const char * type) {
  if (strcmp(type, _state) == 0) {
    return OBS_STATE;
  }
  if (strcmp(type, _sensor) == 0) {
    return OBS_SENSOR;
  }
  return UNKNOWN_OBS_TYPE;
}

template <class T>
struct ObservableObject {
public:
  ObservableObject(ObservableType objType) : type(objType){};
  typedef std::function<T(void)> ValueProviderFunction;

  const char* name;
  ObservableType type;

  ValueProviderFunction valueProvider;

  JsonDocument toJson() {
    JsonDocument doc;
    doc["name"] = name;
    doc["type"] = observableTypeToStr(type);
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
  Sensor() : ObservableObject<int16_t>(OBS_SENSOR){};
  int pin;
  SensorType sensorType;
};
#endif

#if ENABLE_STATES
struct DeviceState : public ObservableObject<String> {
  DeviceState() : ObservableObject<String>(OBS_STATE){};
};
#endif

#endif