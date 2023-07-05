#ifndef WATCHER_H
#define WATCHER_H

#include <functional>

namespace Watcher {
    class Watcher {
        public:
            virtual bool check() = 0;
            Watcher * next;
            Watcher * previous;        
    };
}

#endif