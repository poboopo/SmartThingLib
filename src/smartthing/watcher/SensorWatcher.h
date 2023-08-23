#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/watcher/callback/WatcherCallback.h"

#define SENSOR_WATCHER_TAG "sensor_watcher"
#define SENSOR_WATCHER_TYPE "sensor"

// todo add treshold for analog sensor
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
                        _oldValue = newValue;
                        callCallbacks(&newValue);
                        return true;
                    }
                }
                return false;
            };
            
            bool callbackAccept(Callback::WatcherCallback<int16_t> * callback, int16_t * value) {
                return callback->triggerValue() < 0 || (callback->triggerValue() >= 0 && (*value) == callback->triggerValue());
            };

            const char * getObservableInfo() {
                return _observable->name;
            };
    };
}

#endif