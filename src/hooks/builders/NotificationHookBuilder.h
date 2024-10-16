#ifndef NOTIFICATION_HOOK_BUILDER_H
#define NOTIFICATION_HOOK_BUILDER_H

#include "hooks/impls/NotificationHook.h"

static const char * NOTIFICATION_HOOK_BUILDER_TAG = "notification_cb_builder";

namespace Hook {
class NotificationHookBuilder {
  public:
    template <class B, typename T>
    static Hook<T>* build(JsonObject doc, bool readOnly) {
      if (doc.size() == 0) {
        LOGGER.error(NOTIFICATION_HOOK_BUILDER_TAG, "Json document is empty!");
        return nullptr;
      }
      const char * message = doc[MESSAGE_FIELD];
      if (message == nullptr || strlen(message) == 0) {
        LOGGER.error(NOTIFICATION_HOOK_BUILDER_TAG, "Message can't be empty");
        return nullptr;
      }

      NotificationHook<B, T> * hook = new NotificationHook<B, T>(message, readOnly);
      const char * type = doc[NOTIFICATION_TYPE_FIELD];
      if (type != nullptr && strlen(type) > 0) {
        hook->setNotificationType(type);
      }
      LOGGER.debug(
        NOTIFICATION_HOOK_BUILDER_TAG,
        "Notification hook created: type=%s, message=%s",
        hook->getNoticationType().c_str(),
        message
      );

      return hook;
    }
    static JsonDocument getTemplate() {
      JsonDocument doc;
      JsonObject msg = doc[MESSAGE_FIELD].to<JsonObject>();
      msg["required"] = true;
      
      JsonObject type = doc[NOTIFICATION_TYPE_FIELD].to<JsonObject>();
      JsonArray ar = type["values"].to<JsonArray>();
      ar.add(NOTIFICATION_INFO);
      ar.add(NOTIFICATION_WARNING);
      ar.add(NOTIFICATION_ERROR);

      return doc;
    }
};
}

#endif