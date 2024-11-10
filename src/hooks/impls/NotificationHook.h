#ifndef NOTIFICATION_HOOK_H
#define NOTIFICATIONS_HOOK_H

#ifdef ARDUINO_ARCH_ESP32
#include <HTTPClient.h>
#endif
#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif
#include <type_traits>

#include "hooks/impls/Hook.h"
#include "settings/SettingsRepository.h"
#include "utils/StringUtils.h"

#define MESSAGE_FIELD "message"
#define NOTIFICATION_TYPE_FIELD "ntfType"

#define NOTIFICATION_INFO "info"
#define NOTIFICATION_WARNING "warning"
#define NOTIFICATION_ERROR "error"

static const char * NOTIFICATION_HOOK_TAG = "notification_hook";

// todo extend http hook
template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
class NotificationHook : public T {
  public:
    NotificationHook(const char * message, bool readOnly)
      : T(NOTIFICATION_HOOK, readOnly), _message(message), _notificationType(NOTIFICATION_INFO) {};
    virtual ~NotificationHook() {};

    void call(V &value) {
      _currentValue = value;
      if (WiFi.isConnected()) {
        createRequestTask();
      } else {
        st_log_error(HTTP_HOOK_TAG, "WiFi not connected!");
      }
    }

    void populateJsonWithCustomValues(JsonDocument &doc, boolean shortJson) const {
      doc[MESSAGE_FIELD] = _message.c_str();
      doc[NOTIFICATION_TYPE_FIELD] = _notificationType.c_str();
    }

    void updateCustom(JsonObject obj) {
      if (obj.containsKey(MESSAGE_FIELD)) {
        _message = obj[MESSAGE_FIELD].as<String>();
      }
      if (obj.containsKey(NOTIFICATION_TYPE_FIELD)) {
        _notificationType = obj[NOTIFICATION_TYPE_FIELD].as<String>();
        if (_notificationType.isEmpty()) {
          _notificationType = NOTIFICATION_INFO;
        }
      }
    }

    void setMessage(const char * message) {
      _message = message;
    }
    void setNotificationType(const char * type) {
      _notificationType = type;
    }
    String getNoticationType() {
      return _notificationType;
    }
    #ifdef ARDUINO_ARCH_ESP32
    TaskHandle_t _requestTask = NULL;
    #endif
    bool _sending = false;
  private:
    String _message;
    String _notificationType;
    V _currentValue;

    void createRequestTask() {
      if (_sending) {
        return;
      }
      #ifdef ARDUINO_ARCH_ESP32
      xTaskCreate(
        [](void *o) {
          NotificationHook *hook = static_cast<NotificationHook *>(o);
          hook->_sending = true;
          hook->sendRequest();
          hook->_sending = false;
          vTaskDelete(hook->_requestTask);
        },
        "notification", 10000, this, 1, &_requestTask);
      #endif
      #ifdef ARDUINO_ARCH_ESP8266
      _sending = true;
      sendRequest(); // todo async
      _sending = false;
      #endif
    }

    void sendRequest() {
      String gtwIp = SettingsRepository.getConfig()[GATEWAY_CONFIG].as<String>();
      if (gtwIp.isEmpty()) {
        st_log_debug(NOTIFICATION_HOOK_TAG, "Gateway ip is missing!");
        return;
      }

      String valueStr = String(_currentValue);
      String messageResolved = replaceValues(_message.c_str(), valueStr);

      JsonDocument doc;
      JsonObject from = doc["device"].to<JsonObject>();
      from["name"] = SmartThing.getName();
      from["type"] = SmartThing.getType();
      from["ip"] = SmartThing.getIp();

      JsonObject notification = doc["notification"].to<JsonObject>();
      notification["message"] = messageResolved;
      notification["type"] = _notificationType;

      String payload;
      serializeJson(doc, payload);
      String url = "http://" + gtwIp + "/api/notification";
      st_log_debug(NOTIFICATION_HOOK_TAG, "Sending notification to [%s]:%s", url.c_str(), payload.c_str());

      HTTPClient client;
      client.setTimeout(2000);
      #ifdef ARDUINO_ARCH_ESP32
      client.begin(url);
      #endif
      #ifdef ARDUINO_ARCH_ESP8266
      WiFiClient wifiClient; // todo global var?
      client.begin(wifiClient, url);
      #endif
      client.addHeader("Content-Type", "application/json");
      int code = client.sendRequest("POST", payload.c_str());
      client.end();

      st_log_debug(NOTIFICATION_HOOK_TAG, "Notification send request finished with code %d", code);
    }

};
#endif