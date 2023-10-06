#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include <ArduinoJson.h>
#include "watcher/Watcher.h"
#include "watcher/SensorWatcher.h"
#include "watcher/DeviceStateWatcher.h"
#include "configurable/ConfigurableObjects.h"
#include "watcher/callback/WatcherCallback.h"
#include "watcher/callback/LambdaCallback.h"
#include "utils/List.h"

namespace Callback {
    class CallbacksManager {
        public:
            void loadFromSettings();
            bool createCallbackFromJson(const char * json);
            bool createCallback(JsonObject observableInfo, JsonObject callbackInfo);
            // replace with JsonArray?
            DynamicJsonDocument callbacksToJson(bool ignoreReadOnly, bool shortJson);

            bool addDeviceStateCallback(const char * name, LambdaCallback<String>::CustomCallback callback, const char * triggerValue);
            bool addDeviceStateCallback(const char * name, LambdaCallback<String>::CustomCallback callback) {
                return addDeviceStateCallback(name, callback, nullptr);
            };
            bool addDeviceStateCallback(const char * name, const char * url, const char * triggerValue, bool readonly);
            bool addDeviceStateCallback(const char * name, const char * url, const char * triggerValue) {
                return addDeviceStateCallback(name, url, triggerValue, true);
            }
            bool addDeviceStateCallback(const char * name, const char * url) {
                return addDeviceStateCallback(name, url, nullptr, true);
            };

            bool addSensorCallback(const char * name, LambdaCallback<int16_t>::CustomCallback callback, int16_t triggerValue);
            bool addSensorCallback(const char * name, LambdaCallback<int16_t>::CustomCallback callback) {
                return addSensorCallback(name, callback, -1);
            };
            bool addSensorCallback(const char * name, const char * url, int16_t triggerValue, bool readonly);
            bool addSensorCallback(const char * name, const char * url, int16_t triggerValue) {
                return addSensorCallback(name, url, triggerValue, true);
            }
            bool addSensorCallback(const char * name, const char * url) {
                return addSensorCallback(name, url, -1, true);
            };
            bool addDeviceStateCallback(const Configurable::DeviceState::DeviceState * state, WatcherCallback<String> * callback);
            bool addSensorCallback(const Configurable::Sensor::Sensor * sensor, WatcherCallback<int16_t> * callback);
            /*
                type - тип наблюдаемого объекта (state, sensor)
                name - имя наблюдаемого объекта
                index - индекс в списке callbackов для данного объекта
            */
            bool deleteCallback(const char * type, const char * name, int16_t index);
            bool updateCallback(const char * json);

            void check();
            DynamicJsonDocument getWatchersInfo();
            DynamicJsonDocument getCallbacksJson(const char * type, const char * name);
            
            void saveCallbacksToSettings();

            int16_t getTotalCallbacksCount() {
                return _callbacksCount;
            }

            DynamicJsonDocument getCallbacksTemplates();
        private:
            List<Watcher<int16_t>> _sensorsWatchers; 
            List<Watcher<String>> _statesWatchers; 
            int _callbacksCount = 0;

            template<typename T>
            void collectInfo(List<Watcher<T>> * list, JsonArray * array);

            template<typename T>
            Watcher<T> * getWatcher(List<Watcher<T>> * list, const Configurable::ConfigurableObject<T> * observable);

            template<typename T>
            Watcher<T> * getWatcherByObservableName(List<Watcher<T>> * list, const char * name);

            template<typename T>
            WatcherCallback<T> * getCallbackFromWatcherList(List<Watcher<T>> * list, const char * name, int16_t callbackIndex);

            template<typename T>
            DynamicJsonDocument getCallbacksJsonFromList(List<Watcher<T>> * list, const char * name);

            template<typename T>
            bool deleteWatcherCallbackFromList(List<Watcher<T>> * list, const char * name, int16_t index);

            template<typename T>
            bool updateWatcherCallbackFromList(List<Watcher<T>> * list, const char * name, int16_t index, const char * json);

            template<typename T>
            bool updateCallback(List<Watcher<T>> * list, const char * name, JsonObject callbackObject);

            template<typename T>
            void checkWatchers(List<Watcher<T>> * list);
    };
}

#endif