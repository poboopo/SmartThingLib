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

const char * const _HTTP_HOOK_TAG = "http_hook";
const char * const _urlHookField = "url";
const char * const _methodHookField = "method";
const char * const _payloadHookField = "payload";

const char * const _methodGet = "GET";
const char * const _methodPost = "POST";
const char * const _methodPut = "PUT";
const char * const _methodPatch = "PATCH";
const char * const _methodDelete = "DELETE";

enum RequestMethod {
  UNKOWN_METHOD,
  GET_METHOD,
  POST_METHOD,
  PUT_METHOD,
  PATCH_METHOD,
  DELETE_METHOD
};

inline const char * requestMethodToStr(RequestMethod method) {
  switch (method) {
    case GET_METHOD:
      return _methodGet;
    case POST_METHOD:
      return _methodPost;
    case PUT_METHOD:
      return _methodPut;
    case PATCH_METHOD:
      return _methodPatch;
    case DELETE_METHOD:
      return _methodDelete;
    default:
      return "";
  }
}

inline RequestMethod requestMethodFromStr(const char * method) {
  if (method == nullptr || strlen(method) == 0) {
    return UNKOWN_METHOD;
  }

  if (strcmp(method, _methodGet) == 0) {
    return GET_METHOD;
  }
  if (strcmp(method, _methodPost) == 0) {
    return POST_METHOD;
  }
  if (strcmp(method, _methodPut) == 0) {
    return PUT_METHOD;
  }
  if (strcmp(method, _methodPatch) == 0) {
    return PATCH_METHOD;
  }
  if (strcmp(method, _methodDelete) == 0) {
    return DELETE_METHOD;
  }

  return UNKOWN_METHOD;
}

template<typename T, CHECK_HOOK_DATA_TYPE>
class HttpHook : public SELECT_HOOK_BASE_CLASS {
  public:
    HttpHook(const char *url, RequestMethod method, const char * payload)
        : SELECT_HOOK_BASE_CLASS(HTTP_HOOK), _url(url), _method(method), _payload(payload) {
      fixUrl();
      if (method == UNKOWN_METHOD) {
        method = GET_METHOD;
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
            tmp = String(_method);
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

      if (doc[_methodHookField].is<JsonVariant>()) {
        int method = doc[_methodHookField].as<int>();
        if (method < GET_METHOD || method > DELETE_METHOD) {
          st_log_error(_HTTP_HOOK_TAG, "Bad method code: %d", method);
        } else {
          _method = static_cast<RequestMethod>(method);
          st_log_debug(_HTTP_HOOK_TAG, "Hook's method was updated to %s", requestMethodToStr(_method));
        }
      }

      if (doc[_payloadHookField].is<const char*>()) {
        _payload = doc[_payloadHookField].as<String>();
        st_log_debug(_HTTP_HOOK_TAG, "Hook's payload was updated to %s", _payload.c_str());
      }
    };
 private:
  String _url;
  RequestMethod _method;
  String _payload;
  int16_t _lastResponseCode = 0;
  T _currentValue; // todo try to remove

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
    st_log_debug(_HTTP_HOOK_TAG, "Sending request [%s] %s :: %s", requestMethodToStr(_method), urlResolved.c_str(), payloadResolved.c_str());

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
    _lastResponseCode = client.sendRequest(requestMethodToStr(_method), payloadResolved.c_str());
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