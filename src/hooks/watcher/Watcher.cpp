#include "hooks/watcher/Watcher.h"
#include "sensors/Sensor.h"

template<>
bool Watcher<NUMBER_SENSOR_TYPE>::check() {
  if (_observable != nullptr) {
    NUMBER_SENSOR_TYPE newValue = _observable->provideValue();
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
bool Watcher<TEXT_SENSOR_TYPE>::check() {
  if (_observable == nullptr) {
    return false;
  }
  TEXT_SENSOR_TYPE newValue = _observable->provideValue();
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
void Watcher<NUMBER_SENSOR_TYPE>::setInitialValue() {
  _oldValue = 0;
}

template<>
void Watcher<TEXT_SENSOR_TYPE>::setInitialValue() {
  _oldValue = "";
}