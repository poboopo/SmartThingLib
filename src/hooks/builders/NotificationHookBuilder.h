#ifndef NOTIFICATION_HOOK_BUILDER_H
#define NOTIFICATION_HOOK_BUILDER_H

#include "hooks/impls/NotificationHook.h"

static const char * _NOTIFICATION_HOOK_BUILDER_TAG = "notification_cb_builder";

class NotificationHookBuilder {
  public:
    template <class B, typename T>
    static Hook<T>* build(JsonObject doc, bool readOnly) {
      if (doc.size() == 0) {
        st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Json document is empty!");
        return nullptr;
      }
      const char * message = doc[_messageHookField];
      if (message == nullptr || strlen(message) == 0) {
        st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Message can't be empty");
        return nullptr;
      }

      NotificationHook<B, T> * hook = new NotificationHook<B, T>(message, readOnly);
      const char * type = doc[_nftHookField];
      if (type != nullptr && strlen(type) > 0) {
        hook->setNotificationType(type);
      }
      st_log_debug(
        _NOTIFICATION_HOOK_BUILDER_TAG,
        "Notification hook created: type=%s, message=%s",
        hook->getNoticationType().c_str(),
        message
      );

      return hook;
    }
    static JsonDocument getTemplate() {
      JsonDocument doc;
      JsonObject msg = doc[_messageHookField].to<JsonObject>();
      msg["required"] = true;
      
      JsonObject type = doc[_nftHookField].to<JsonObject>();
      JsonArray ar = type["values"].to<JsonArray>();
      ar.add(NOTIFICATION_INFO);
      ar.add(NOTIFICATION_WARNING);
      ar.add(NOTIFICATION_ERROR);

      return doc;
    }
};

#endif