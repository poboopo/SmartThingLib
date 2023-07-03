#ifndef WATCHER_H
#define WATCHER_H

#include <functional>

namespace Watcher {
    // pass argument with void *?
    typedef std::function<void(void)> Callback;

    class Watcher {
        public:
            virtual bool check() = 0;
            Watcher * next;
            Watcher * previous;
    };
}

#endif