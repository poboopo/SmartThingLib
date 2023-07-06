#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/logs/BetterLogger.h"

#define SENSOR_WATCHER_TAG "sensor_watcher"

namespace Watcher {
    class SensorWatcher: public Watcher {
        public:
        // todo add treshold for analog sensor
            SensorWatcher(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<uint16_t> * callback): 
                _sensor(sensor), _oldValue(0), _callback(callback){};
            bool check() {
                if (_sensor != nullptr) {
                    uint16_t newValue = _sensor->valueGenerator();
                    if (newValue != _oldValue) {
                        LOGGER.debug(
                            SENSOR_WATCHER_TAG, 
                            "Sensor %s value changed %d->%d. Calling callback.", 
                            _sensor->name, _oldValue, newValue
                        );
                        _callback->call(&newValue);
                        _oldValue = newValue;
                        return true;
                    }
                }
                return false;
            };

            StaticJsonDocument<WATCHERS_INFO_DOC_SIZE> getInfo() {
                StaticJsonDocument<WATCHERS_INFO_DOC_SIZE> doc;
                doc["type"] = SENSOR_WATCHER_TAG;
                doc["sensorName"] = _sensor->name;
                doc["callback"] = _callback->getInfo();
                return doc;
            };
        protected:
            const Configurable::Sensor::Sensor * _sensor;
            uint16_t _oldValue;
            Callback::WatcherCallback<uint16_t> * _callback;
    };
}

#endif