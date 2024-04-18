#ifndef HTTP_HOOK_H
#define HTTP_HOOK_H

#include <HTTPClient.h>
#include <type_traits>

#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"

#define HTTP_HOOK_TAG "http_hook"

namespace Hook {
template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
class HttpHook : public T {
 public:
  HttpHook(const char *url, bool readonly)
      : T(HTTP_HOOK_TAG, readonly), _url(url), _method("GET") {
    fixUrl();
  };
  void call(V &value) {
    _currentValue = value;
    if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
      createRequestTask();
    } else {
      LOGGER.error(HTTP_HOOK_TAG, "WiFi not connected!");
    }
  };

  void addCustomJsonValues(DynamicJsonDocument doc, boolean shortJson) {
    if (!shortJson) {
      doc["lastResponseCode"] = _lastResponseCode;
    }
    doc["url"] = _url;
    doc["method"] = _method;
    doc["payload"] = _payload;
  };

  void updateCustom(JsonObject doc) {
    if (doc.containsKey("url")) {
      _url = doc["url"].as<String>();
      fixUrl();
      LOGGER.debug(HTTP_HOOK_TAG, "Hook's url was updated to %s",
                   _url.c_str());
    }
    if (doc.containsKey("method")) {
      _method = doc["method"].as<String>();
      LOGGER.debug(HTTP_HOOK_TAG, "Hook's method was updated to %s",
                   _method.c_str());
    }
    if (doc.containsKey("payload")) {
      _payload = doc["payload"].as<String>();
      LOGGER.debug(HTTP_HOOK_TAG, "Hook's payload was updated to %s",
                   _payload.c_str());
    }
  };
  void setPayload(const char *payload) { _payload = payload; }
  void setMethod(const char *method) { _method = method; }

  TaskHandle_t _requestTask = NULL;  // not safe
  bool _sending = false;

 private:
  String _url;
  String _method;
  String _payload;
  int16_t _lastResponseCode = 0;
  V _currentValue;

  void createRequestTask() {
    if (_sending) {
      LOGGER.warning(HTTP_HOOK_TAG, "Request task already exist! Skipping");
      return;
    }
    xTaskCreate(
        [](void *o) {
          HttpHook *hook = static_cast<HttpHook *>(o);
          hook->_sending = true;
          hook->sendRequest();
          hook->_sending = false;
          vTaskDelete(hook->_requestTask);
        },
        _url.c_str(), 10000, this, 1, &_requestTask);
  }

  void sendRequest() {
    // probably bad realisation for memory

    String valueStr = String(_currentValue);

    String urlCopy = _url;
    String payloadCopy = _payload;

    LOGGER.debug(HTTP_HOOK_TAG, "Replacing ${v} with [%s]",
                 valueStr.isEmpty() ? "blank_value" : valueStr.c_str());
    urlCopy.replace(VALUE_DYNAMIC_PARAM, valueStr);
    payloadCopy.replace(VALUE_DYNAMIC_PARAM, valueStr);
    LOGGER.info(HTTP_HOOK_TAG, "Sending request [%s] %s :: %s",
                _method.c_str(), urlCopy.c_str(), payloadCopy.c_str());

    HTTPClient client;
    client.setTimeout(2000);
    client.begin("http://" + urlCopy);
    if (!payloadCopy.isEmpty()) {
      client.addHeader("Content-Type", "application/json");
    }
    _lastResponseCode =
        client.sendRequest(_method.c_str(), payloadCopy.c_str());
    client.end();

    LOGGER.info(HTTP_HOOK_TAG, "Request %s finished with code %d",
                urlCopy.c_str(), _lastResponseCode);
  }

  void fixUrl() {
    _url.trim();
    if (_url.startsWith("http://")) {
      _url.remove(0, 7);
    }
  }
};
}  // namespace Hook
#endif