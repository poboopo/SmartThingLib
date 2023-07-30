#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include <ArduinoJson.h>
#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/SensorWatcher.h"
#include "smartthing/watcher/DeviceStateWatcher.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/utils/List.h"

#define WATCHERS_LIST_INFO_SIZE 512

namespace Watcher {
    class WatchersManager {
        public:
            bool addDeviceStateCallback(const Configurable::DeviceState::DeviceState * state, Callback::WatcherCallback<char *> * callback);
            bool addSensorCallback(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<int16_t> * callback);

            void check();
            DynamicJsonDocument getWatchersInfo();
            DynamicJsonDocument getWatcherCallbacksInfo(const char * type, const char * name);
        private:
            List<Watcher<Configurable::Sensor::Sensor, int16_t>> _sensorsWatchers; 
            List<Watcher<Configurable::DeviceState::DeviceState, char *>> _statesWatchers; 

            template<typename O, typename T>
            void collectInfo(List<Watcher<O, T>> * list, JsonArray * array);

            template<typename O, typename T>
            Watcher<O, T> * getWatcher(List<Watcher<O, T>> * list, const O * observable);

            template<typename O, typename T>
            void checkWatchers(List<Watcher<O, T>> * list);
    };
}

#endif