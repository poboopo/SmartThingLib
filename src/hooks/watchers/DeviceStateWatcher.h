#ifndef DEVICE_STATE_WATCHER_H
#define DEVICE_STATE_WATCHER_H

#include "Features.h"
#if ENABLE_STATES

#include "hooks/impls/Hook.h"
#include "hooks/watchers/Watcher.h"
#include "observable/ObservableObjects.h"
#include "logs/BetterLogger.h"

#define DEVICE_STATE_WATCHER_TAG "device_state_watcher"

#define STATE_WATCHER_TYPE "state"

namespace Hook {
class DeviceStateWatcher : public Watcher<String> {
 public:
  DeviceStateWatcher(const Observable::DeviceState::DeviceState* deviceState)
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
      // LOGGER.debug(DEVICE_STATE_WATCHER_TAG,
      //              "Device state [%s] value changed %s->%s.", _observable->name,
      //              _oldValue, newValue);
      callHooks(newValue);

      _oldValue = newValue;
      return true;
    }
    return false;
  };

  const char* getObservableInfo() { return _observable->name; };
};
}  // namespace Hook

#endif
#endif