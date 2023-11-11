#ifndef DEVICE_STATE_WATCHER_H
#define DEVICE_STATE_WATCHER_H

#include "watcher/Watcher.h"
#include "watcher/callback/WatcherCallback.h"
#include "configurable/ConfigurableObjects.h"
#include "logs/BetterLogger.h"
#include "watcher/callback/WatcherCallback.h"

#define DEVICE_STATE_WATCHER_TAG "device_state_watcher"

using namespace Configurable::Sensor;
#define STATE_WATCHER_TYPE "state"

namespace Callback {
    class DeviceStateWatcher: public Watcher<String> {
        public:
            DeviceStateWatcher(const Configurable::DeviceState::DeviceState * deviceState, Callback::WatcherCallback<String> * callback): 
                Watcher<String>(deviceState, callback, "") {};
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
                    callCallbacks(newValue);

                    _oldValue = newValue;
                    return true;
                }
                return false;
            };
            
            const char * getObservableInfo() {
                return _observable->name;
            };
    };
}

#endif