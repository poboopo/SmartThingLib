#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/configurable/ConfigurableObjects.h"

namespace Watcher {
    class WatchersList {
        public:
            WatchersList(): _head(nullptr){};
            ~WatchersList();

            bool registerSensorWatcher(const Configurable::Sensor::Sensor * sensor, Callback::Sensor::Callback callback);
            bool registerDeviceStateWatcher(const Configurable::DeviceState::DeviceState * state, Callback::DeviceState::Callback callback);

            void check();
        private:
            Watcher * _head;
            int _count = 0;
            void append(Watcher * watcher);
    };
}

#endif