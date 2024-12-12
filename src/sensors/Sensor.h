#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "Features.h"

#include <ArduinoJson.h>
#include <functional>

#define NUMBER_SENSOR_TYPE int
#define TEXT_SENSOR_TYPE String

const char * const _state = "state";
const char * const _sensor = "sensor";

template <typename T>
class Sensor {
  public:
    typedef std::function<T(void)> ValueProvider;

    Sensor(const char * name, std::function<T(void)> valueProvider): 
      _name(name), _valueProvider(valueProvider) {};

    const char * name() const {
      return _name;
    }

    T provideValue() const {
      return _valueProvider();
    }
  private:
    const char* _name; // todo copy string
    ValueProvider _valueProvider;
};

#endif