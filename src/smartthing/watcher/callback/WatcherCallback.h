#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>

#define CALLBACK_INFO_DOC_SIZE 128

namespace Watcher {
    namespace Callback {
        template<typename T>
        class WatcherCallback {
            public:
                WatcherCallback(T triggerValue, bool readonly): _triggerValue(triggerValue), _readonly(readonly){};
                // todo make value const
                virtual void call(T * value);
                virtual StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> toJson() = 0;
                const T triggerValue() const {
                    return _triggerValue;
                };
                StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> getDeaultInfo() {
                    StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> doc;
                    doc["trigger"] = _triggerValue;
                    doc["readonly"] = _readonly;
                    return doc;
                }
            protected:
                T _triggerValue;
                bool _readonly;
        };
    }
}

#endif