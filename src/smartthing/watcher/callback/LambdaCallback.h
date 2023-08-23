#ifndef LAMBDA_CALLBACK_H
#define LAMBDA_CALLBACK_H

#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/logs/BetterLogger.h"
#include <functional>

#define LAMBDA_CALLBACK_TAG "lambda_callback"

namespace Callback {
    template<typename T>
    class LambdaCallback: public WatcherCallback<T> {
        public:
            typedef std::function<void(T *)>CustomCallback;

            LambdaCallback(CustomCallback customCallback, T triggerValue):
                WatcherCallback<T>(LAMBDA_CALLBACK_TAG, triggerValue, true), _customCallback(customCallback) {};
            LambdaCallback(CustomCallback customCallback, T triggerValue, bool readOnly):
                WatcherCallback<T>(LAMBDA_CALLBACK_TAG, triggerValue, readOnly), _customCallback(customCallback) {};
            void call(T * value) {
                if (value == nullptr) {
                    LOGGER.error(LAMBDA_CALLBACK_TAG, "Value is null!");
                    return;
                }
                _customCallback(value);
            };
            DynamicJsonDocument toJson(bool shortJson) {
                DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE);
                doc["caption"] = "lambda";
                this->addDefaultInfo(doc);
                return doc;
            };
            void updateCustom(JsonObject doc){};
        private:
            CustomCallback _customCallback;
    };
}
#endif