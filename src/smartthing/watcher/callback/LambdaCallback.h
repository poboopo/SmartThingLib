#ifndef LAMBDA_CALLBACK_H
#define LAMBDA_CALLBACK_H

#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/logs/BetterLogger.h"
#include <functional>

#define LAMBDA_CALLBACK_TAG "lambda_callback"

namespace Watcher {
    namespace Callback {
        template<typename T>
        class LambdaCallback: public WatcherCallback<T> {
            public:
                typedef std::function<void(T *)>CustomCallback;

                LambdaCallback(CustomCallback customCallback): _customCallback(customCallback) {};
                void call(T * value) {
                    if (value == nullptr) {
                        LOGGER.error(LAMBDA_CALLBACK_TAG, "Value is null!");
                        return;
                    }
                    _customCallback(value);
                }
            private:
                CustomCallback _customCallback;
        };
    }
}
#endif