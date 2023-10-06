#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "watcher/Watcher.h"
#include "watcher/callback/WatcherCallback.h"
#include "configurable/ConfigurableObjects.h"
#include "logs/BetterLogger.h"
#include "watcher/callback/WatcherCallback.h"

#define SENSOR_WATCHER_TAG "sensor_watcher"
#define SENSOR_WATCHER_TYPE "sensor"

// todo add threshold for analog sensor
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
            
            bool callbackAccept(Callback::WatcherCallback<int16_t> * callback, int16_t &oldValue, int16_t &value) {
                bool result = false;
                if (callback->triggerValue() >= 0) {
                    LOGGER.debug(SENSOR_WATCHER_TAG, "Trigger value present: %d", callback->triggerValue());
                    result = callback->triggerValue() >= 0 && value == callback->triggerValue();
                } else if (callback->thresholdValue() > 0) {
                    int diff = abs(_oldValue - value);
                    result = diff > callback->thresholdValue();
                    if (!result) {
                        LOGGER.debug(
                            SENSOR_WATCHER_TAG, 
                            "Diff (%d) less then callback's threshold (%d)",
                            diff,
                            callback->thresholdValue()
                        );
                    }
                } else {
                    LOGGER.debug(SENSOR_WATCHER_TAG, "No trigger or threshvalue present, accepting");
                    result = true;
                }
                return result;
            };

            const char * getObservableInfo() {
                return _observable->name;
            };
    };
}

#endif