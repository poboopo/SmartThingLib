#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/logs/BetterLogger.h"

namespace Watcher {
    class SensorWatcher: public Watcher {
        public:
        // todo add treshold for analog sensor
            SensorWatcher(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<uint16_t> * callback): 
                _observable(sensor), _oldValue(0), _callback(callback){};
            bool check() {
                if (_observable != nullptr) {
                    uint16_t newValue = _observable->valueGenerator();
                    if (newValue != _oldValue) {
                        LOGGER.debug(
                            "sensor_watcher", 
                            "Sensor %s value changed %d->%d. Calling callback.", 
                            _observable->name, _oldValue, newValue
                        );
                        _callback->call(&newValue);
                        _oldValue = newValue;
                        return true;
                    }
                }
                return false;
            };
        protected:
            const Configurable::Sensor::Sensor * _observable;
            uint16_t _oldValue;
            Callback::WatcherCallback<uint16_t> * _callback;
    };
}

#endif