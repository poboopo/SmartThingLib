#ifndef HTTP_CALLBACK_BUILDER_H
#define HTTP_CALLBACK_BUILDER_H

#include "CallbackBuilder.h"
#include "callbacks/impls/HttpCallback.h"
#include "logs/BetterLogger.h"

#define HTTP_CALLBACK_BUILDER_TAG "http_cb_builder"

#define HTTP_CALLBACKS_TEMPLATES_JSON                                      \
  "{\"url\": {\"required\": true},\"method\": {\"required\": "             \
  "false,\"values\": [\"GET\",\"POST\",\"DELETE\",\"PUT\"]},\"payload\": " \
  "{\"required\": false}}"

namespace Callback {
class HttpCallbackBuilder : public CallbackBuilder {
 public:
  template <typename T>
  static Callback<T>* build(JsonObject doc, bool readOnly) {
    if (doc.size() == 0) {
      LOGGER.error(HTTP_CALLBACK_BUILDER_TAG, "Json document is empty!");
      return nullptr;
    }
    const char* url = doc["url"];
    if (url == nullptr || strlen(url) == 0) {
      LOGGER.error(HTTP_CALLBACK_BUILDER_TAG, "Url can't be blank!");
      return nullptr;
    }
    String method = doc["method"];
    if (method.isEmpty()) {
      method = "GET";
    }
    const char* payload = doc["payload"];

    HttpCallback<T>* callback = new HttpCallback<T>(url, readOnly);
    callback->setPayload(payload);
    callback->setMethod(method.c_str());
    LOGGER.debug(HTTP_CALLBACK_BUILDER_TAG,
                 "Http callback created: url=%s, method=%s, payload=%s", url,
                 method.c_str(), payload == nullptr ? "-" : payload);

    ::Callback::CallbackBuilder::defaultValues(callback, doc);

    return callback;
  }
  static DynamicJsonDocument getTemplate() {
    DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
    deserializeJson(doc, HTTP_CALLBACKS_TEMPLATES_JSON);
    return doc;
  }
};
}  // namespace Callback

#endif