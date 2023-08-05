#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>

#define CALLBACK_INFO_DOC_SIZE 128
#define JSON_TRIGGER_FIELD "trigger"

namespace Callback {
    template<typename T>
    class WatcherCallback {
        public:
            WatcherCallback(const char * type, T triggerValue, bool readonly): 
                _type(type), _triggerValue(triggerValue), _readonly(readonly){};
            // todo make value const
            virtual void call(T * value) = 0;
            virtual StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> toJson() = 0;
            virtual void updateCustom(DynamicJsonDocument doc) = 0;

            StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> getDeaultInfo() {
                StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> doc;
                doc[JSON_TRIGGER_FIELD] = _triggerValue;
                doc["readonly"] = _readonly;
                doc["type"] = _type;
                return doc;
            }
            
            T * triggerValuePointer() {
                return &_triggerValue;
            }

            bool isReadonly() {
                return _readonly;
            }
            const T triggerValue() const {
                return _triggerValue;
            }
        protected:
            const char * _type;
            T _triggerValue;
            bool _readonly;
    };
}

#endif