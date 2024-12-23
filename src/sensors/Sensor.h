#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include "Features.h"

#include <ArduinoJson.h>
#include <functional>

// todo move to sensor manager
#ifndef NUMBER_SENSOR_DATA_TYPE
  #define NUMBER_SENSOR_DATA_TYPE int
#endif
#define TEXT_SENSOR_DATA_TYPE String

const char * const _state = "state";
const char * const _sensor = "sensor";

template <typename T>
class Sensor {
  public:
    typedef std::function<T(void)> ValueProvider;

    Sensor(const char * name, ValueProvider valueProvider): 
      _valueProvider(valueProvider) {
        _name = (char *) malloc(strlen(name) + 1);
        strcpy(_name, name);
      };
    ~Sensor() {
      free(_name);
    }

    const char * name() const {
      return _name;
    }

    T provideValue() const {
      return _valueProvider();
    }
  private:
    char* _name; // todo copy string
    ValueProvider _valueProvider;
};

#endif