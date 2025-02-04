#include "hooks/watcher/Watcher.h"
#include "sensors/Sensor.h"

#if ENABLE_HOOKS
  #if ENABLE_NUMBER_SENSORS
    template<>
    bool Watcher<NUMBER_SENSOR_DATA_TYPE>::check() {
      if (_sensor != nullptr) {
        NUMBER_SENSOR_DATA_TYPE newValue = _sensor->provideValue();
        if (_oldValue == -1) {
          _oldValue = newValue;
          return false;
        }
        if (newValue != _oldValue) {
          callHooks(newValue);
          _oldValue = newValue;
          return true;
        }
      }
      return false;
    }

    template<>
    void Watcher<NUMBER_SENSOR_DATA_TYPE>::setInitialValue() {
      _oldValue = 0;
    }
  #endif

  #if ENABLE_TEXT_SENSORS
    template<>
    bool Watcher<TEXT_SENSOR_DATA_TYPE>::check() {
      if (_sensor == nullptr) {
        return false;
      }
      TEXT_SENSOR_DATA_TYPE newValue = _sensor->provideValue();
      if (_oldValue.isEmpty()) {
        _oldValue = newValue;
        return false;
      }

      if (!newValue.equals(_oldValue)) {
        callHooks(newValue);

        _oldValue = newValue;
        return true;
      }
      return false;
    }
    template<>
    void Watcher<TEXT_SENSOR_DATA_TYPE>::setInitialValue() {
      _oldValue = "";
    }
  #endif
#endif
