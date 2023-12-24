#ifndef HTTP_CALLBACK_BUILDER_H
#define HTTP_CALLBACK_BUILDER_H

#include "CallbackBuilder.h"
#include "logs/BetterLogger.h"
#include "callbacks/impls/HttpCallback.h"

#define HTTP_CALLBACK_BUILDER_TAG "http_cb_builder"

namespace Callback {
  template<typename T>
  class HttpCallbackBuilder: public CallbackBuilder<T> {
    public:
      static Callback<T> * build(JsonObject doc, bool readOnly) {
        if (doc.size() == 0) {
          LOGGER.error(HTTP_CALLBACK_BUILDER_TAG, "Json document is empty!");
          return nullptr;
        }
        const char * url = doc["url"];
        if (url == nullptr || strlen(url) == 0) {
          LOGGER.error(HTTP_CALLBACK_BUILDER_TAG, "Url can't be blank!");
          return nullptr;
        }
        const char * method = doc["method"];
        const char * payload = doc["payload"];

        HttpCallback<T> * callback = new HttpCallback<T>(url, readOnly);
        callback->setPayload(payload);
        callback->setMethod(method);
        LOGGER.debug(
          HTTP_CALLBACK_BUILDER_TAG,
          "Http callback created: url=%s, method=%s, payload=%s",
          url,
          method == nullptr ? "-" : method,
          payload == nullptr ? "-" : payload
        );

        ::Callback::CallbackBuilder<T>::defaultValues(callback, doc);

        return callback;
      }
  };
}

#endif