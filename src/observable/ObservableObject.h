#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "Features.h"

#include <ArduinoJson.h>
#include <functional>

#define NUMBER_SENSOR_TYPE int
#define TEXT_SENSOR_TYPE String

const char * const _state = "state";
const char * const _sensor = "sensor";

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

template <typename T>
class ObservableObject {
  public:
    typedef std::function<T(void)> ValueProvider;

    ObservableObject(const char * name, std::function<T(void)> valueProvider): 
      _name(name), _valueProvider(valueProvider) {};

    const char * name() const {
      return _name;
    }

    T provideValue() const {
      return _valueProvider();
    }

    ObservableType type() const;

    JsonDocument toJson() {
      JsonDocument doc;
      doc["name"] = _name;
      doc["type"] = observableTypeToStr(type());
      return doc;
    };

    String toString() const {
      String res;
      res += type();
      res += _name;
      return res;
    }
  private:
    const char* _name; // todo copy string
    ValueProvider _valueProvider;
};

#endif