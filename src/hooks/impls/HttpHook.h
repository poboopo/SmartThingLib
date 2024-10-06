#ifndef HTTP_HOOK_H
#define HTTP_HOOK_H

#ifdef ARDUINO_ARCH_ESP32
#include <HTTPClient.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif
#include <type_traits>

#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"
#include "utils/StringUtils.h"

#define HTTP_HOOK_TAG "http_hook"

namespace Hook {
template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
class HttpHook : public T {
 public:
  HttpHook(const char *url, bool readonly)
      : T(HTTP_HOOK_TAG, readonly), _url(url), _method("GET") {
    fixUrl();
  };
  virtual ~HttpHook() {};

  void call(V &value) {
    _currentValue = value;
    if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
      createRequestTask();
    } else {
      LOGGER.error(HTTP_HOOK_TAG, "WiFi not connected!");
    }
  };

  void addCustomJsonValues(JsonDocument &doc, boolean shortJson) {
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

  #ifdef ARDUINO_ARCH_ESP32
  TaskHandle_t _requestTask = NULL;  // not safe
  #endif
  bool _sending = false;

 private:
  String _url;
  String _method;
  String _payload;
  int16_t _lastResponseCode = 0;
  V _currentValue;

  void createRequestTask() {
    if (_sending) {
      LOGGER.debug(HTTP_HOOK_TAG, "Request task already exist! Skipping");
      return;
    }
    #ifdef ARDUINO_ARCH_ESP32
    xTaskCreate(
      [](void *o) {
        HttpHook *hook = static_cast<HttpHook *>(o);
        hook->_sending = true;
        hook->sendRequest();
        hook->_sending = false;
        vTaskDelete(hook->_requestTask);
      },
      _url.c_str(), 10000, this, 1, &_requestTask);
    #endif
    #ifdef ARDUINO_ARCH_ESP8266
    _sending = true;
    sendRequest(); // todo make async
    _sending = false;
    #endif
  }

  void sendRequest() {
    String valueStr = String(_currentValue);
    String urlResolved = replaceValues(_url.c_str(), valueStr);
    String payloadResolved = replaceValues(_payload.c_str(), valueStr);

    LOGGER.debug(HTTP_HOOK_TAG, "Resolved url and payload: %s, %s", urlResolved.c_str(), payloadResolved.c_str());
    LOGGER.debug(HTTP_HOOK_TAG, "Sending request [%s] %s :: %s", _method.c_str(), urlResolved.c_str(), payloadResolved.c_str());

    HTTPClient client;
    client.setTimeout(2000);
    #ifdef ARDUINO_ARCH_ESP32
    client.begin("http://" + urlResolved);
    #endif
    #ifdef ARDUINO_ARCH_ESP8266
    WiFiClient wifiClient; // todo global var?
    client.begin(wifiClient, "http://" + urlResolved);
    #endif
    if (!payloadResolved.isEmpty()) {
      client.addHeader("Content-Type", "application/json");
    }
    _lastResponseCode = client.sendRequest(_method.c_str(), payloadResolved.c_str());
    client.end();

    LOGGER.info(HTTP_HOOK_TAG, "Request %s finished with code %d", urlResolved.c_str(), _lastResponseCode);
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