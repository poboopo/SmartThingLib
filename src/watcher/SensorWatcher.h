#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "watcher/Watcher.h"
#include "watcher/callback/WatcherCallback.h"
#include "configurable/ConfigurableObjects.h"
#include "logs/BetterLogger.h"
#include "watcher/callback/WatcherCallback.h"

#define SENSOR_WATCHER_TAG "sensor_watcher"
#define SENSOR_WATCHER_TYPE "sensor"

namespace Callback {
    class SensorWatcher: public Watcher<int16_t> {
        public:
            SensorWatcher(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<int16_t> * callback): 
                Watcher<int16_t>(sensor, callback, -1) {};
            bool check() {
                if (_observable != nullptr) {
                    int16_t newValue = _observable->valueGenerator();
                    if (_oldValue == -1) {
                        _oldValue = newValue;
                        return false;
                    }
                    if (newValue != _oldValue) {
                        LOGGER.debug(
                            SENSOR_WATCHER_TAG, 
                            "Sensor %s value changed %d->%d.", 
                            _observable->name, _oldValue, newValue
                        );
                        callCallbacks(_oldValue, newValue);
                        _oldValue = newValue;
                        return true;
                    }
                }
                return false;
            };
        
            const char * getObservableInfo() {
                return _observable->name;
            };
    };
}

#endif