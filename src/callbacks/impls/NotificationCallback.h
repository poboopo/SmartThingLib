#ifndef NOTIFICATION_CALLBACK_H
#define NOTIFICATIONS_CALLBACK_H

#include "callbacks/impls/Callback.h"
#include "settings/SettingsManager.h"

#define NOTIFICATION_CALLBACK_TAG "notification_callback"
#define MESSAGE_FIELD "message"
#define NOTIFICATION_TYPE_FIELD "ntfType"

#define NOTIFICATION_INFO "info"
#define NOTIFICATION_WARNING "warning"
#define NOTIFICATION_ERROR "error"

#define NOTIFIACTION_PATH "/notification"

namespace Callback {
template <typename T>
class NotificationCallback : public Callback<T> {
  public:
    NotificationCallback(const char * message, bool readOnly)
      : Callback<T>(NOTIFICATION_CALLBACK_TAG, readOnly), _message(message), _notificationType(NOTIFICATION_INFO) {};

    void call(T &value) {
      _ip = STSettings.getConfig()[GATEWAY_CONFIG].as<String>();
      if (_ip.isEmpty()) {
        LOGGER.debug(NOTIFICATION_CALLBACK_TAG, "Gateway ip is missing!");
        return;
      }
      _currentValue = value;
      if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
        createRequestTask();
      } else {
        LOGGER.error(HTTP_CALLBACK_TAG, "WiFi not connected!");
      }
    }
    DynamicJsonDocument toJson(bool shortJson) {
      DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE);
      doc[MESSAGE_FIELD] = _message.c_str();
      doc[NOTIFICATION_TYPE_FIELD] = _notificationType.c_str();
      this->addDefaultInfo(doc);
      return doc;
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
    T _currentValue;

    void createRequestTask() {
      if (_sending) {
        return;
      }
      xTaskCreate(
        [](void *o) {
          NotificationCallback *callback = static_cast<NotificationCallback *>(o);
          callback->_sending = true;
          callback->sendRequest();
          callback->_sending = false;
          vTaskDelete(callback->_requestTask);
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
      LOGGER.debug(NOTIFICATION_CALLBACK_TAG, "Notification payload: %s", payload.c_str());

      String url = "http://" + _ip + "/notification";

      LOGGER.debug(NOTIFICATION_CALLBACK_TAG, "Sending notification to [%s]:%s", url.c_str(), payload.c_str());

      HTTPClient client;
      client.setTimeout(2000);
      client.begin(url);
      client.addHeader("Content-Type", "application/json");
      int code = client.sendRequest("POST", payload.c_str());
      client.end();

      LOGGER.debug(NOTIFICATION_CALLBACK_TAG, "Notification send finished with code %d", code);
    }

};
}

#endif