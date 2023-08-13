#ifndef CALLBACK_BUILDER_H
#define CALLBACK_BUILDER_H

#include "smartthing/watcher/callback/LambdaCallback.h"
#include "smartthing/watcher/callback/HttpCallback.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/logs/BetterLogger.h"

#define CALLBACK_BUILDER_TAG "callback_builder"

namespace Callback {
    class CallbackBuilder {
        public:
            CallbackBuilder(): _isReadOnly(false){};

            template<typename T>
            WatcherCallback<T> * build(T triggerValue) {
                WatcherCallback<T> * callback;
                if (_url != nullptr) {
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Building new http callback %s", _url.c_str());
                    callback = new HttpCallback<T>(_url.c_str(), _method.c_str(), _payload.c_str(), triggerValue, _isReadOnly);
                } else {
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Building new lambda callback");
                    callback = new LambdaCallback<T>([](T * v){}, triggerValue);
                }
                return callback;
            }

            // CallbackBuilder * callback(LambdaCallback::CustomCallback * callback) {
            //     if (callback == nullptr) {
            //         return this;
            //     }
            //     _callback = callback;
            //     return this;
            // }
            CallbackBuilder * readOnly(bool isReadOnly) {
                _isReadOnly = isReadOnly;
                return this;
            }
            CallbackBuilder * url(const char * url) {
                if (url == nullptr) {
                    return this;
                }
                _url = url;
                return this;
            }
            CallbackBuilder * method(const char * method) {
                if (method == nullptr) {
                    return this;
                }
                _method = method;
                return this;
            }
            CallbackBuilder * payload(const char * payload) {
                if (payload == nullptr) {
                    return this;
                }
                _payload = payload;
                return this;
            }
            
        private:
            // LambdaCallback<int16_t>::CustomCallback * _callback;
            bool _isReadOnly;
            // replace with String?
            String _url;
            String _method;
            String _payload;
    };
}

#endif