#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>

#define CALLBACK_INFO_DOC_SIZE 512
#define JSON_TRIGGER_FIELD "trigger"
#define MAX_CALLBACK_TEMAPLATE_SIZE 1024

//todo RENAME TO CALLBACK

namespace Callback {
    template<typename T>
    // template<ConfigurableObject<T> ????>
    class WatcherCallback {
        public:
            WatcherCallback(const char * type, T triggerValue, bool readonly): 
                _type(type), _triggerValue(triggerValue), _readonly(readonly){};
            // todo make value const
            virtual void call(T * value) = 0;
            virtual DynamicJsonDocument toJson(bool shortJson) = 0;
            virtual void updateCustom(JsonObject doc) = 0;

            void addDefaultInfo(DynamicJsonDocument& doc) {
                doc[JSON_TRIGGER_FIELD] = _triggerValue;
                doc["readonly"] = _readonly;
                doc["type"] = _type;
            }
            
            void setTriggerValue(T triggerValue) {
                _triggerValue = triggerValue;
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