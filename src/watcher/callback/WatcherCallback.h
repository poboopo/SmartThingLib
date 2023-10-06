#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>

#define CALLBACK_INFO_DOC_SIZE 512
#define MAX_CALLBACK_TEMAPLATE_SIZE 1024

namespace Callback {
    template<typename T>
    // template<ConfigurableObject<T> ????>
    class WatcherCallback {
        public:
            WatcherCallback(const char * type, T triggerValue, bool readonly): 
                _type(type), _triggerValue(triggerValue), _threshold(-1), _readonly(readonly){};

            WatcherCallback(const char * type, T triggerValue, int16_t threshold, bool readonly): 
                _type(type), _triggerValue(triggerValue), _threshold(threshold), _readonly(readonly){};
            // todo make value const
            virtual void call(T * value) = 0;
            virtual DynamicJsonDocument toJson(bool shortJson) = 0;
            virtual void updateCustom(JsonObject doc) = 0;

            void addDefaultInfo(DynamicJsonDocument& doc) {
                // check with typeinfo?
                doc["trigger"] = _triggerValue;
                if (_threshold >= 0) {
                    doc["threshold"] = _threshold;
                }
                doc["readonly"] = _readonly;
                doc["type"] = _type;
            }

            void setTriggerValue(T triggerValue) {
                _triggerValue = triggerValue;
            }
            void setThresholdValue(int16_t value) {
                _threshold = value;
            }

            bool isReadonly() {
                return _readonly;
            }
            const T triggerValue() const {
                return _triggerValue;
            }
            const int16_t thresholdValue() const {
                return _threshold;
            }

            const char * type() const {
                return _type;
            }
        protected:
            const char * _type;
            T _triggerValue;
            int16_t _threshold;
            bool _readonly;
    };
}

#endif