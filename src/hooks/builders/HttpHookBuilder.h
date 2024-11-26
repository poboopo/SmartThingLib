#ifndef HTTP_HOOK_BUILDER_H
#define HTTP_HOOK_BUILDER_H

#include "hooks/impls/HttpHook.h"
#include "logs/BetterLogger.h"

#define HTTP_HOOKS_TEMPLATES_JSON                                      \
  "{\"url\": {\"required\": true},\"method\": {\"required\": "             \
  "false,\"values\": [\"GET\",\"POST\",\"DELETE\",\"PUT\"]},\"payload\": " \
  "{\"required\": false}}"

static const char * _HTTP_HOOK_BUILDER_TAG = "http_cb_builder";

class HttpHookBuilder {
 public:
  template <typename T>
  static Hook<T>* build(JsonDocument doc) {
    return build<T>(doc[_urlHookField], doc[_methodHookField], doc[_payloadHookField]);
  }

  template <typename T>
  static Hook<T> * build(const char * data) {
    String url, method, payload;
    uint8_t step = 0;

    for (int i = 0; i < strlen(data); i++) {
      if (data[i] == ';') {
        step++;
        continue;
      }

      switch (step) {
        case 0:
          url += data[i];
          break;
        case 1:
          method += data[i];
          break;
        case 2:
          payload += data[i];
          break;
      }
    }

    url.replace("|;", ";");
    method.replace("|;", ";");
    payload.replace("|;", ";");

    return build<T>(url.c_str(), method.c_str(), payload.c_str());
  }

  template<typename T>
  static Hook<T> * build(const char * url, const char * method, const char * payload) {
    if (url == nullptr || strlen(url) == 0) {
      st_log_error(_HTTP_HOOK_BUILDER_TAG, "Url can't be empty!");
      return nullptr;
    }

    st_log_debug(
      _HTTP_HOOK_BUILDER_TAG,
      "Http hook data:url=%s,method=%s,payload=%s",
      url,
      method == nullptr || strlen(method) == 0 ? "GET" : method,
      payload == nullptr ? "[empty]" : payload
    );

    return new HttpHook<T>(url, method, payload);
  }

  static JsonDocument getTemplate() {
    JsonDocument doc;
    deserializeJson(doc, HTTP_HOOKS_TEMPLATES_JSON);
    return doc;
  }
};

#endif