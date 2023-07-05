#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/watcher/callback/WatcherCallback.h"

namespace Watcher {
    class WatchersManager {
        public:
            WatchersManager(): _head(nullptr){};
            ~WatchersManager();

            bool registerDeviceStateWatcher(const Configurable::DeviceState::DeviceState * state, Callback::WatcherCallback<const char *> * callback);
            bool registerSensorWatcher(const Configurable::Sensor::Sensor * sensor, Callback::WatcherCallback<uint16_t> * callback);

            void check();
        private:
            Watcher * _head;
            int _count = 0;
            void append(Watcher * watcher);
    };
}

#endif