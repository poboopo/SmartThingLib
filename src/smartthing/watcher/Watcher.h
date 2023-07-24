#ifndef WATCHER_H
#define WATCHER_H

#include <functional>
#include <ArduinoJson.h>

#define WATCHERS_INFO_DOC_SIZE 128

namespace Watcher {
    class Watcher {
        public:
            virtual bool check() = 0;
            virtual StaticJsonDocument<WATCHERS_INFO_DOC_SIZE> getInfo() = 0;;
            Watcher * next;
            Watcher * previous;
    };
}

#endif