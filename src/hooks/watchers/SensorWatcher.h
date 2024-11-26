#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "Features.h"
#if ENABLE_SENSORS 

#include "hooks/impls/Hook.h"
#include "hooks/watchers/Watcher.h"
#include "observable/ObservableObjects.h"
#include "logs/BetterLogger.h"

#define SENSOR_WATCHER_TYPE "sensor"

class SensorWatcher : public Watcher<int16_t> {
 public:
  SensorWatcher(const Sensor* sensor)
      : Watcher<int16_t>(sensor, -1){};
  virtual ~SensorWatcher() {};

  bool check() {
    if (_observable != nullptr) {
      int16_t newValue = _observable->valueProvider();
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
  };
};
#endif
#endif