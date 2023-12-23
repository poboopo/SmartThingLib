#ifndef LAMBDA_CALLBACK_H
#define LAMBDA_CALLBACK_H

#include "callbacks/impls/WatcherCallback.h"
#include "logs/BetterLogger.h"
#include <functional>

#define LAMBDA_CALLBACK_TAG "lambda_callback"

namespace Callback {
    template<class T>
    class LambdaCallback: public WatcherCallback<T> {
        public:
            typedef std::function<void(T &value)>CustomCallback;

            LambdaCallback(CustomCallback customCallback, T triggerValue):
                WatcherCallback<T>(LAMBDA_CALLBACK_TAG, triggerValue, true), _customCallback(customCallback) {};
            LambdaCallback(CustomCallback customCallback, T triggerValue, bool readOnly):
                WatcherCallback<T>(LAMBDA_CALLBACK_TAG, triggerValue, readOnly), _customCallback(customCallback) {};
            void call(T &value) {
                _customCallback(value);
            };
            DynamicJsonDocument toJson(bool shortJson) {
                DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE);
                this->addDefaultInfo(doc);
                return doc;
            };
            void updateCustom(JsonObject doc){};
        private:
            CustomCallback _customCallback;
    };
}
#endif