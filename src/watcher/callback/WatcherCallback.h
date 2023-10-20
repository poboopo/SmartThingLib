#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>
#include <functional>
#include "watcher/comparator/Comparator.h"
#include "logs/BetterLogger.h"

#define CALLBACK_INFO_DOC_SIZE 512
#define MAX_CALLBACK_TEMPLATE_SIZE 1024

namespace Callback {
    const String DEFAULT_CALLBACKS_TEMPLATES_JSON = R"=====(
    {
        "triggerDisabled": {
            "type": "boolean",
            "required": true,
            "values": [true, false],
            "default": true
        },
        "trigger": {
            "required": false
        },
        "compareType": {
            "required": true,
            "values": [
                "eq",
                "neq",
                "gte",
                "lte"
            ],
            "default": "eq"
        }
    }
    )=====";

    template<typename T>
    // template<ConfigurableObject<T> ????>
    class WatcherCallback {
        public:
            WatcherCallback(const char * type, T triggerValue, bool readonly): 
                _type(type), _triggerValue(triggerValue), _readonly(readonly), _compareType(EQ){};

            // todo make value const
            virtual void call(T * value) = 0;
            virtual DynamicJsonDocument toJson(bool shortJson) = 0;
            virtual void updateCustom(JsonObject doc) = 0;

            bool accept(T &value) {
                return _triggerDisabled || Comparator::compare(_compareType, value, _triggerValue);
            }

            void addDefaultInfo(DynamicJsonDocument& doc) {
                doc["readonly"] = _readonly;
                doc["type"] = _type;
                doc["trigger"] = _triggerValue;
                doc["triggerDisabled"] = _triggerDisabled;
                doc["compareType"] = compareTypeToString(_compareType);
            }

            void setCompareType(String type) {
                _compareType = compareTypeFromString(type, CompareType::EQ);
            }
            void setCompareType(CompareType type) {
                _compareType = type;
            }
            void setTriggerDisabled(bool disabled) {
                _triggerDisabled = disabled;
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
            const char * type() const {
                return _type;
            }

            static DynamicJsonDocument getTemplate() {
                DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
                deserializeJson(doc, DEFAULT_CALLBACKS_TEMPLATES_JSON);
                return doc;
            }
        protected:
            const char * _type;
            T _triggerValue;
            bool _triggerDisabled;
            bool _readonly;
            CompareType _compareType;
    };
}

#endif