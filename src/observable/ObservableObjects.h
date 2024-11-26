#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "Features.h"

#include <ArduinoJson.h>
#include <functional>

#define SENSOR_DATA_TYPE int16_t
#define STATE_DATA_TYPE String

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

  String toString() const {
    String res;
    res += type;
    res += name;
    return res;
  }
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

struct Sensor : public ObservableObject<SENSOR_DATA_TYPE> {
public:
  Sensor() : ObservableObject<SENSOR_DATA_TYPE>(OBS_SENSOR){};
  int pin;
  SensorType sensorType;
};
#endif

#if ENABLE_STATES
struct DeviceState : public ObservableObject<STATE_DATA_TYPE> {
  DeviceState() : ObservableObject<STATE_DATA_TYPE>(OBS_STATE){};
};
#endif

#endif