#ifndef DEVICE_STATE_WATCHER_H
#define DEVICE_STATE_WATCHER_H

#include "Features.h"
#if ENABLE_STATES

#include "hooks/impls/Hook.h"
#include "hooks/watchers/Watcher.h"
#include "observable/ObservableObjects.h"
#include "logs/BetterLogger.h"

#define STATE_WATCHER_TYPE "state"

class DeviceStateWatcher : public Watcher<String> {
 public:
  DeviceStateWatcher(const DeviceState* deviceState)
      : Watcher<String>(deviceState, ""){};
  virtual ~DeviceStateWatcher() {};

  bool check() {
    if (_observable == nullptr) {
      return false;
    }
    String newValue = _observable->valueProvider();
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
  };
};

#endif
#endif