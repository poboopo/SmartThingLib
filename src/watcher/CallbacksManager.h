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
            String createCallbackFromJson(const char * json);
            String createCallback(JsonObject observableInfo, JsonObject callbackInfo);
            // replace with JsonArray?
            DynamicJsonDocument callbacksToJson(bool ignoreReadOnly, bool shortJson);

            String addDeviceStateCallback(const char * name, LambdaCallback<String>::CustomCallback callback, const char * triggerValue);
            String addDeviceStateCallback(const char * name, LambdaCallback<String>::CustomCallback callback) {
                return addDeviceStateCallback(name, callback, nullptr);
            };
            String addDeviceStateCallback(const char * name, const char * url, const char * triggerValue, bool readonly);
            String addDeviceStateCallback(const char * name, const char * url, const char * triggerValue) {
                return addDeviceStateCallback(name, url, triggerValue, true);
            }
            String addDeviceStateCallback(const char * name, const char * url) {
                return addDeviceStateCallback(name, url, nullptr, true);
            };

            String addSensorCallback(const char * name, LambdaCallback<int16_t>::CustomCallback callback, int16_t triggerValue);
            String addSensorCallback(const char * name, LambdaCallback<int16_t>::CustomCallback callback) {
                return addSensorCallback(name, callback, -1);
            };
            String addSensorCallback(const char * name, const char * url, int16_t triggerValue, bool readonly);
            String addSensorCallback(const char * name, const char * url, int16_t triggerValue) {
                return addSensorCallback(name, url, triggerValue, true);
            }
            String addSensorCallback(const char * name, const char * url) {
                return addSensorCallback(name, url, -1, true);
            };
            String addDeviceStateCallback(const Configurable::DeviceState::DeviceState * state, WatcherCallback<String> * callback);
            String addSensorCallback(const Configurable::Sensor::Sensor * sensor, WatcherCallback<int16_t> * callback);
            /*
                type - тип наблюдаемого объекта (state, sensor)
                name - имя наблюдаемого объекта
                index - индекс в списке callbackов для данного объекта
            */
            bool deleteCallback(const char * type, const char * name, String id);
            bool updateCallback(const char * json);

            void check();
            DynamicJsonDocument getWatchersInfo();
            DynamicJsonDocument getCallbacksJson(const char * type, const char * name);
            DynamicJsonDocument getCallbackJsonById(const char * type, const char * name, String id);
            
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
            WatcherCallback<T> * getCallbackFromWatcherList(List<Watcher<T>> * list, const char * name, String id);

            template<typename T>
            DynamicJsonDocument getCallbacksJsonFromList(List<Watcher<T>> * list, const char * name);

            template<typename T>
            DynamicJsonDocument getCallbackJsonFromList(List<Watcher<T>> * list, const char * name, String id);

            template<typename T>
            bool deleteWatcherCallbackFromList(List<Watcher<T>> * list, const char * name, String id);

            template<typename T>
            bool updateCallback(List<Watcher<T>> * list, const char * name, JsonObject callbackObject);

            template<typename T>
            void checkWatchers(List<Watcher<T>> * list);
    };
}

#endif