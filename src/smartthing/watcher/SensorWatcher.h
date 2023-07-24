#ifndef SENSOR_WATCHER_H
#define SENSOR_WATCHER_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/logs/BetterLogger.h"

#define SENSOR_WATCHER_TAG "sensor_watcher"
#define SENSOR_WATCHER_TYPE SENSOR_WATCHER_TAG

namespace Watcher {
    class SensorWatcher: public Watcher {
        public:
        // todo add treshold for analog sensor
            SensorWatcher(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<int16_t> * callback): 
                _sensor(sensor), _oldValue(0), _callback(callback){};
            bool check() {
                if (_sensor != nullptr) {
                    int16_t newValue = _sensor->valueGenerator();
                    if (newValue != _oldValue) {
                        LOGGER.debug(
                            SENSOR_WATCHER_TAG, 
                            "Sensor %s value changed %d->%d.", 
                            _sensor->name, _oldValue, newValue
                        );
                        if (_callback->triggerValue() < 0 || (_callback->triggerValue() >= 0 && newValue == _callback->triggerValue())) {
                            _callback->call(&newValue);
                        }
                        _oldValue = newValue;
                        return true;
                    }
                }
                return false;
            };

            StaticJsonDocument<WATCHERS_INFO_DOC_SIZE> getInfo() {
                StaticJsonDocument<WATCHERS_INFO_DOC_SIZE> doc;
                doc["type"] = SENSOR_WATCHER_TYPE;
                doc["observable"] = _sensor->name;
                doc["callback"] = _callback->getInfo();
                if (_callback->triggerValue() > 0) {
                    doc["trigger"] = _callback->triggerValue();
                }
                return doc;
            };
        protected:
            const Configurable::Sensor::Sensor * _sensor;
            int16_t _oldValue;
            Callback::WatcherCallback<int16_t> * _callback;
    };
}

#endif