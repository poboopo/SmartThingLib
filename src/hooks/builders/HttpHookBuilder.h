#ifndef HTTP_HOOK_BUILDER_H
#define HTTP_HOOK_BUILDER_H

#include "hooks/impls/HttpHook.h"
#include "logs/BetterLogger.h"

#define HTTP_HOOK_BUILDER_TAG "http_cb_builder"

#define HTTP_HOOKS_TEMPLATES_JSON                                      \
  "{\"url\": {\"required\": true},\"method\": {\"required\": "             \
  "false,\"values\": [\"GET\",\"POST\",\"DELETE\",\"PUT\"]},\"payload\": " \
  "{\"required\": false}}"

namespace Hook {
class HttpHookBuilder {
 public:
  template <class B, typename T>
  static Hook<T>* build(JsonObject doc, bool readOnly) {
    if (doc.size() == 0) {
      LOGGER.error(HTTP_HOOK_BUILDER_TAG, "Json document is empty!");
      return nullptr;
    }
    const char* url = doc["url"];
    if (url == nullptr || strlen(url) == 0) {
      LOGGER.error(HTTP_HOOK_BUILDER_TAG, "Url can't be blank!");
      return nullptr;
    }
    String method = doc["method"];
    if (method.isEmpty()) {
      method = "GET";
    }
    const char* payload = doc["payload"];

    HttpHook<B, T>* hook = new HttpHook<B, T>(url, readOnly);
    hook->setPayload(payload);
    hook->setMethod(method.c_str());
    LOGGER.debug(HTTP_HOOK_BUILDER_TAG,
                 "Http hook created: url=%s, method=%s, payload=%s", url,
                 method.c_str(), payload == nullptr ? "-" : payload);

    return hook;
  }
  static DynamicJsonDocument getTemplate() {
    DynamicJsonDocument doc(MAX_HOOK_TEMPLATE_SIZE);
    deserializeJson(doc, HTTP_HOOKS_TEMPLATES_JSON);
    return doc;
  }
};
}  // namespace Hook

#endif