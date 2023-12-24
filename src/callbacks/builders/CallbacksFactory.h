#ifndef CALLBACKS_FACTORY_H
#define CALLBACKS_FACTORY_H

#include <ArduinoJson.h>

#include "callbacks/builders/ActionCallbackBuilder.h"
#include "callbacks/builders/HttpCallbackBuilder.h"

#include "callbacks/impls/Callback.h"
#include "callbacks/impls/ActionCallback.h"
#include "callbacks/impls/HttpCallback.h"

#define CALLBACKS_FACTORY_TAG "callbacks_factory"

namespace Callback {
  class CallbacksFactory {
    public:
      template<typename T>
      static Callback<T> * build(JsonObject doc) {
        const char * type = doc["type"];
        if (type == nullptr) {
          LOGGER.error(CALLBACKS_FACTORY_TAG, "Callback type is missing!");
          return nullptr;
        }

        LOGGER.debug(CALLBACKS_FACTORY_TAG, "----------------------BUILDER-START----------------------");
        LOGGER.debug(CALLBACKS_FACTORY_TAG, "Building callback type=%s", type);

        Callback<T> * callback = nullptr;
        if (strcmp(type, ACTION_CALLBACK_TAG) == 0) {
          callback = ActionCallbackBuilder<T>::build(doc, false);
        } else if (strcmp(type, HTTP_CALLBACK_TAG) == 0) {
          callback = HttpCallbackBuilder<T>::build(doc, false);
        } else {
          LOGGER.error(CALLBACKS_FACTORY_TAG, "Unkonwn callback type: %s", type);
        }
        if (callback == nullptr) {
          LOGGER.debug(CALLBACKS_FACTORY_TAG, "Build failed");
        }

        LOGGER.debug(CALLBACKS_FACTORY_TAG, "-----------------------BUILDER-END----------------------");
        return callback;
      }
  };
}

#endif