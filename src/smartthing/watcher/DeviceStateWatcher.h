#ifndef DEVICE_STATE_WATCHER_H
#define DEVICE_STATE_WATCHER_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/watcher/callback/WatcherCallback.h"

#define DEVICE_STATE_WATCHER_TAG "device_state_watcher"

using namespace Configurable::Sensor;
#define STATE_WATCHER_TYPE "state"

namespace Callback {
    class DeviceStateWatcher: public Watcher<String> {
        public:
            DeviceStateWatcher(const Configurable::DeviceState::DeviceState * deviceState, Callback::WatcherCallback<String> * callback): 
                Watcher<String>(deviceState, callback) {
                    _oldValue = "";
                };
            bool check() {
                if (_observable == nullptr) {
                    return false;
                }

                String newValue = _observable->valueGenerator();
                if (_oldValue.isEmpty()) {
                    _oldValue = newValue;
                    return false;
                }
                
                if (!newValue.equals(_oldValue)) {
                    LOGGER.debug(
                        DEVICE_STATE_WATCHER_TAG, 
                        "Device state %s value changed %s->%s.", 
                        _observable->name, _oldValue, newValue
                    );

                    _oldValue = newValue;
                    callCallbacks(&_oldValue);

                    return true;
                }
                return false;
            };

            bool callbackAccept(Callback::WatcherCallback<String> * callback, String * value) {
                return callback->triggerValue() == nullptr || 
                    (callback->triggerValue() != nullptr && strcmp((*value).c_str(), callback->triggerValue().c_str()) == 0);
            }

            const char * getObservableInfo() {
                return _observable->name;
            };
    };
}

#endif