#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include <ArduinoJson.h>
#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/SensorWatcher.h"
#include "smartthing/watcher/DeviceStateWatcher.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/utils/List.h"

namespace Callback {
    class CallbacksManager {
        public:
            bool addDeviceStateCallback(const Configurable::DeviceState::DeviceState * state, Callback::WatcherCallback<char *> * callback);
            bool addSensorCallback(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<int16_t> * callback);
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
        private:
            List<Watcher<Configurable::Sensor::Sensor, int16_t>> _sensorsWatchers; 
            List<Watcher<Configurable::DeviceState::DeviceState, char *>> _statesWatchers; 

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