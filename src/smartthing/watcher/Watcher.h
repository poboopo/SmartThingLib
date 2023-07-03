#ifndef WATCHER_H
#define WATCHER_H

#include <functional>

namespace Watcher {
    namespace Callback {
        namespace Sensor {
            typedef std::function<void(uint16_t)> Callback;
        }
        namespace DeviceState {
            typedef std::function<void(const char *)> Callback;
        }
    }
    class Watcher {
        public:
            virtual bool check() = 0;
            Watcher * next;
            Watcher * previous;
    };
}

#endif