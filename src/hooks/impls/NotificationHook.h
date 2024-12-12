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

#include "SmartThing.h"
#include "hooks/impls/Hook.h"
#include "settings/SettingsRepository.h"
#include "utils/StringUtils.h"

const char * const _NOTIFICATION_HOOK_TAG = "notification_hook";
const char * const _messageHookField = "message";
const char * const _nftHookField =  "notificationType";

const char * const _notificationInfoStr = "info";
const char * const _notificationWarningStr = "warning";
const char * const _notificationErrorStr = "error";

enum NotificationType {
  NOTIFICATION_UNKNOWN,
  NOTIFICATION_INFO,
  NOTIFICATION_WARNING,
  NOTIFICATION_ERROR
};

inline const char * notificationTypeToStr(NotificationType type) {
  switch (type) {
    case NOTIFICATION_INFO:
      return _notificationInfoStr;
    case NOTIFICATION_WARNING:
      return _notificationWarningStr;
    case NOTIFICATION_ERROR:
      return _notificationErrorStr;
    default:
      return "unknown";
  }
}

inline NotificationType notificationTypeFromStr(const char * type, NotificationType defaultValue = NOTIFICATION_UNKNOWN) {
  if (type == nullptr) {
    return defaultValue;
  }

  if (strcmp(_notificationInfoStr, type) == 0) {
    return NOTIFICATION_INFO;
  }
  if (strcmp(_notificationWarningStr, type) == 0) {
    return NOTIFICATION_WARNING;
  }
  if (strcmp(_notificationErrorStr, type) == 0) {
    return NOTIFICATION_ERROR;
  }

  return defaultValue;
}

// todo extend http hook
template<typename T, CHECK_HOOK_DATA_TYPE>
class NotificationHook : public SELECT_HOOK_BASE_CLASS {
  public:
    NotificationHook(NotificationType notificationType, const char * message)
      : SELECT_HOOK_BASE_CLASS(NOTIFICATION_HOOK), _message(message), _notificationType(notificationType) {};
    virtual ~NotificationHook() {};

    void call(T &value) {
      _currentValue = value;
      if (WiFi.isConnected()) {
        createRequestTask();
      } else {
        st_log_error(_NOTIFICATION_HOOK_TAG, "WiFi not connected!");
      }
    }

    String getMessage() {
      return _message;
    }
    void setMessage(const char * message) {
      _message = message;
    }
    void setNotificationType(NotificationType type) {
      _notificationType = type;
    }
    NotificationType getNoticationType() {
      return _notificationType;
    }
    #ifdef ARDUINO_ARCH_ESP32
    TaskHandle_t _requestTask = NULL;
    #endif
    bool _sending = false;

  protected:
    String customValuesString() {
      String tmp = _message;
      tmp.replace(";", "|;");
      char buff[2 + tmp.length()];
      sprintf(buff, "%d%s", _notificationType, tmp.c_str());
      tmp = buff;
      return tmp;
    }

    void populateJsonWithCustomValues(JsonDocument &doc) const {
      doc[_messageHookField] = _message.c_str();
      doc[_nftHookField] = _notificationType;
    }

    void updateCustom(JsonDocument &doc) {
      if (doc[_messageHookField].is<const char*>()) {
        _message = doc[_messageHookField].as<String>();
      }
      if (doc[_nftHookField].is<JsonVariant>()) {
        int type = doc[_nftHookField].as<int>();
        if (type < NOTIFICATION_INFO || type > NOTIFICATION_ERROR) {
          st_log_error(_NOTIFICATION_HOOK_TAG, "Unknown notification type: %d", type);
        } else {
          _notificationType = static_cast<NotificationType>(type);
        }
      }
    }
    
  private:
    String _message;
    NotificationType _notificationType; // todo enum
    T _currentValue;

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
        st_log_debug(_NOTIFICATION_HOOK_TAG, "Gateway ip is missing!");
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
      notification["type"] = notificationTypeToStr(_notificationType);

      String payload;
      serializeJson(doc, payload);
      String url = "http://" + gtwIp + "/api/notification";
      st_log_debug(_NOTIFICATION_HOOK_TAG, "Sending notification to [%s]:%s", url.c_str(), payload.c_str());

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

      st_log_debug(_NOTIFICATION_HOOK_TAG, "Notification send request finished with code %d", code);
    }

};
#endif