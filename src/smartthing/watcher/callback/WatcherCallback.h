#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>

#define INFO_DOC_SIZE 128

namespace Watcher {
    namespace Callback {
        template<typename T>
        class WatcherCallback {
            public:
                WatcherCallback(){};
                virtual void call(T * value);
                virtual StaticJsonDocument<INFO_DOC_SIZE> getInfo() = 0;
        };
    }
}

#endif