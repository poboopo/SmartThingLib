#ifndef CALLBACK_BUILDER_H
#define CALLBACK_BUILDER_H

#include <typeinfo>

#include "callbacks/impls/LambdaCallback.h"
#include "callbacks/impls/HttpCallback.h"
#include "callbacks/impls/ActionCallback.h"
#include "callbacks/impls/WatcherCallback.h"
#include "logs/BetterLogger.h"

#define CALLBACK_BUILDER_TAG "callback_builder"

namespace Callback {
    class CallbackBuilder {
        public:
            CallbackBuilder(): _readOnly(false), _compareType(CompareType::EQ), _triggerDisabled(true) {};

            template<typename T>
            WatcherCallback<T> * build(T triggerValue) {
                if (_type.isEmpty()) {
                    LOGGER.error(CALLBACK_BUILDER_TAG, "Callback type is missing! Can't build allback without it.");
                    return nullptr;
                }

                LOGGER.debug(CALLBACK_BUILDER_TAG, "----------------------BUILDER-START----------------------");
                LOGGER.debug(CALLBACK_BUILDER_TAG, "Trying to build callback of type [%s]", _type.c_str());

                WatcherCallback<T> * callback = nullptr;
                if (_type.equals(HTTP_CALLBACK_TAG)) {
                    if (_url.isEmpty()) {
                        LOGGER.error(CALLBACK_BUILDER_TAG, "Can't build callback of type %s without url!", _type.c_str());
                        return nullptr;
                    }
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Building new http callback %s", _url.c_str());
                    callback = new HttpCallback<T>(_url.c_str(), _method.c_str(), _payload.c_str(), triggerValue, _readOnly);
                } else if (_type.equals(LAMBDA_CALLBACK_TAG)) {
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Building new lambda callback");
                    callback = new LambdaCallback<T>([](T &v){}, triggerValue, _readOnly);
                } else if (_type.equals(ACTION_CALLBACK_TAG)) {
                    if (_action.isEmpty()) {
                        LOGGER.error(CALLBACK_BUILDER_TAG, "Can't build callback of type %s without action!", _action.c_str());
                        return nullptr;
                    }
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Building new action callback");
                    callback = new ActionCallback<T>(_action.c_str(), triggerValue, _readOnly);
                } else {
                    LOGGER.error(CALLBACK_BUILDER_TAG, "Failed to build callback of type %s", _type.c_str());
                    return nullptr;
                }

                if (!_id.isEmpty() && !_id.equals("null") && !_id.equals("New")) {
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Callback id: %s", _id.c_str());
                    callback->setId(_id.toInt());
                } else {
                    LOGGER.debug(CALLBACK_BUILDER_TAG, "Callback id is blank");
                    callback->setId(-1);
                }

                if (_compareType.isEmpty()) {
                    _compareType = "eq";
                }

                LOGGER.debug(CALLBACK_BUILDER_TAG, "Compare type: %s", _compareType.c_str());
                callback->setCompareType(_compareType);
                LOGGER.debug(CALLBACK_BUILDER_TAG, "Trigger disabled: %s", _triggerDisabled ? "true" : "false");
                callback->setTriggerDisabled(_triggerDisabled);
                LOGGER.debug(CALLBACK_BUILDER_TAG, "-----------------------BUILDER-END----------------------");

                return callback;
            }
            
            CallbackBuilder * id(String id) {
                _id = id;
                return this;
            }

            CallbackBuilder * readOnly(bool isReadOnly) {
                _readOnly = isReadOnly;
                return this;
            }
            CallbackBuilder * triggerDisabled(bool disabled) {
                _triggerDisabled = disabled;
                return this;
            }
            CallbackBuilder * type(String type) {
                _type = type;
                return this;
            }
            CallbackBuilder * url(String url) {
                _url = url;
                return this;
            }
            CallbackBuilder * method(String method) {
                _method = method;
                return this;
            }
            CallbackBuilder * payload(String payload) {
                _payload = payload;
                return this;
            }
            CallbackBuilder * action(String action) {
                _action = action;
                return this;
            }
            CallbackBuilder * compareType(String compareType) {
                _compareType = compareType;
                return this;
            }
        private:
            String _id;
            bool _readOnly;
            JsonObject _triggerValue;
            String _type;
            String _url;
            String _method;
            String _payload;
            String _action;
            String _compareType;
            bool _triggerDisabled;
    };
}

#endif