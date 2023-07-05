#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

namespace Watcher {
    namespace Callback {
        template<typename T>
        class WatcherCallback {
            public:
                WatcherCallback(){};
                virtual void call(T * value);
        };
    }
}

#endif