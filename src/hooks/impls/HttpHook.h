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

static const char * _HTTP_HOOK_TAG = "http_hook";
static const char * _urlHookField = "url";
static const char * _methodHookField = "method";
static const char * _payloadHookField = "payload";

template<class T, CHECK_HOOK_DATA_TYPE>
class HttpHook : public SELECT_HOOK_BASE_CLASS {
  public:
    HttpHook(const char *url, const char * method, const char * payload)
        : SELECT_HOOK_BASE_CLASS(HTTP_HOOK), _url(url), _method(method), _payload(payload) {
      fixUrl();
      if (method == nullptr || strlen(method) == 0) {
        method = "GET";
      } 
    };
    virtual ~HttpHook() {};

    void call(T &value) {
      _currentValue = value;
      if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
        createRequestTask();
      } else {
        st_log_error(_HTTP_HOOK_TAG, "WiFi not connected!");
      }
    };

    void setPayload(const char *payload) { _payload = payload; }
    void setMethod(const char *method) { _method = method; }

    #ifdef ARDUINO_ARCH_ESP32
    TaskHandle_t _requestTask = NULL;  // not safe todo setter?
    #endif
    bool _sending = false;
    
  protected:
    String customValuesString() {
      String tmp;
      String res;

      for (int i = 0; i < 3; i++) {
        switch(i) {
          case 0:
            tmp = _url;
            break;
          case 1:
            tmp = _method;
            break;
          case 2:
            tmp = _payload;
            break;
        }
        tmp.replace(";", "|;");
        res += tmp;
        res += ';';
      }
      
      return res;
    }

    void populateJsonWithCustomValues(JsonDocument &doc) const {
      doc["lastResponseCode"] = _lastResponseCode;
      doc[_urlHookField] = _url;
      doc[_methodHookField] = _method;
      doc[_payloadHookField] = _payload;
    };

    void updateCustom(JsonDocument &doc) {
      if (doc[_urlHookField].is<const char*>()) {
        _url = doc[_urlHookField].as<String>();
        fixUrl();
        st_log_debug(_HTTP_HOOK_TAG, "Hook's url was updated to %s",
                    _url.c_str());
      }
      if (doc[_methodHookField].is<const char*>()) {
        _method = doc[_methodHookField].as<String>();
        st_log_debug(_HTTP_HOOK_TAG, "Hook's method was updated to %s",
                    _method.c_str());
      }
      if (doc[_payloadHookField].is<const char*>()) {
        _payload = doc[_payloadHookField].as<String>();
        st_log_debug(_HTTP_HOOK_TAG, "Hook's payload was updated to %s",
                    _payload.c_str());
      }
    };
 private:
  String _url;
  String _method; // todo enum
  String _payload;
  int16_t _lastResponseCode = 0;
  T _currentValue;

  void createRequestTask() {
    if (_sending) {
      st_log_debug(_HTTP_HOOK_TAG, "Request task already exist! Skipping");
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

    st_log_debug(_HTTP_HOOK_TAG, "Resolved url and payload: %s, %s", urlResolved.c_str(), payloadResolved.c_str());
    st_log_debug(_HTTP_HOOK_TAG, "Sending request [%s] %s :: %s", _method.c_str(), urlResolved.c_str(), payloadResolved.c_str());

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

    st_log_info(_HTTP_HOOK_TAG, "Request %s finished with code %d", urlResolved.c_str(), _lastResponseCode);
  }

  void fixUrl() {
    _url.trim();
    if (_url.startsWith("http://")) {
      _url.remove(0, 7);
    }
  }
};
#endif