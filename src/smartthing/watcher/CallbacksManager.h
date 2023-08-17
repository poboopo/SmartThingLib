#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include <ArduinoJson.h>
#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/SensorWatcher.h"
#include "smartthing/watcher/DeviceStateWatcher.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/watcher/callback/LambdaCallback.h"
#include "smartthing/utils/List.h"

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
            bool updateCallback(const char * type, const char * name, int16_t index, const char * json);

            void check();
            DynamicJsonDocument getWatchersInfo();
            DynamicJsonDocument getCallbacksJson(const char * type, const char * name);
            
            void saveCallbacksToSettings();
        private:
            List<Watcher<Configurable::Sensor::Sensor, int16_t>> _sensorsWatchers; 
            List<Watcher<Configurable::DeviceState::DeviceState, String>> _statesWatchers; 
            int _callbacksCount = 0;

            template<typename O, typename T>
            void collectInfo(List<Watcher<O, T>> * list, JsonArray * array);

            template<typename O, typename T>
            Watcher<O, T> * getWatcher(List<Watcher<O, T>> * list, const O * observable);

            template<typename O, typename T>
            Watcher<O, T> * getWatcherByObservableName(List<Watcher<O, T>> * list, const char * name);

            template<typename O, typename T>
            WatcherCallback<T> * getCallbackFromWatcherList(List<Watcher<O, T>> * list, const char * name, int16_t callbackIndex);

            template<typename O, typename T>
            DynamicJsonDocument getCallbacksJsonFromList(List<Watcher<O, T>> * list, const char * name);

            template<typename O, typename T>
            bool deleteWatcherCallbackFromList(List<Watcher<O, T>> * list, const char * name, int16_t index);

            template<typename O, typename T>
            bool updateWatcherCallbackFromList(List<Watcher<O, T>> * list, const char * name, int16_t index, const char * json);

            template<typename O, typename T>
            void checkWatchers(List<Watcher<O, T>> * list);
    };
}

#endif