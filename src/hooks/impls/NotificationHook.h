#ifndef NOTIFICATION_HOOK_H
#define NOTIFICATIONS_HOOK_H

#include <type_traits>
#include "hooks/impls/Hook.h"
#include "settings/SettingsManager.h"

#define NOTIFICATION_HOOK_TAG "notification_hook"
#define MESSAGE_FIELD "message"
#define NOTIFICATION_TYPE_FIELD "ntfType"

#define NOTIFICATION_INFO "info"
#define NOTIFICATION_WARNING "warning"
#define NOTIFICATION_ERROR "error"

#define NOTIFIACTION_PATH "/notification"

namespace Hook {
template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
class NotificationHook : public T {
  public:
    NotificationHook(const char * message, bool readOnly)
      : T(NOTIFICATION_HOOK_TAG, readOnly), _message(message), _notificationType(NOTIFICATION_INFO) {};

    void call(V &value) {
      _ip = STSettings.getConfig()[GATEWAY_CONFIG].as<String>();
      if (_ip.isEmpty()) {
        LOGGER.debug(NOTIFICATION_HOOK_TAG, "Gateway ip is missing!");
        return;
      }
      _currentValue = value;
      if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
        createRequestTask();
      } else {
        LOGGER.error(HTTP_HOOK_TAG, "WiFi not connected!");
      }
    }

    void addCustomJsonValues(JsonDocument &doc, boolean shortJson) {
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

    TaskHandle_t _requestTask = NULL;
    bool _sending = false;
  private:
    String _message;
    String _ip;
    String _notificationType;
    V _currentValue;

    void createRequestTask() {
      if (_sending) {
        return;
      }
      xTaskCreate(
        [](void *o) {
          NotificationHook *hook = static_cast<NotificationHook *>(o);
          hook->_sending = true;
          hook->sendRequest();
          hook->_sending = false;
          vTaskDelete(hook->_requestTask);
        },
        "notification", 10000, this, 1, &_requestTask);
    }

    void sendRequest() {
      String valueStr = String(_currentValue);

      String messageCopy = _message;
      messageCopy.replace(VALUE_DYNAMIC_PARAM, valueStr);

      StaticJsonDocument<256> doc;
      JsonObject from = doc.createNestedObject("device");
      from["name"] = SmartThing.getName();
      from["type"] = SmartThing.getType();
      from["ip"] = SmartThing.getIp();

      JsonObject notification = doc.createNestedObject("notification");
      notification["message"] = messageCopy;
      notification["type"] = _notificationType;

      String payload;
      serializeJson(doc, payload);
      String url = "http://" + _ip + "/notification";
      LOGGER.debug(NOTIFICATION_HOOK_TAG, "Sending notification to [%s]:%s", url.c_str(), payload.c_str());

      HTTPClient client;
      client.setTimeout(2000);
      client.begin(url);
      client.addHeader("Content-Type", "application/json");
      int code = client.sendRequest("POST", payload.c_str());
      client.end();

      LOGGER.debug(NOTIFICATION_HOOK_TAG, "Notification send finished with code %d", code);
    }

};
}

#endif