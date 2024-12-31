#ifndef NOTIFICATION_HOOK_BUILDER_H
#define NOTIFICATION_HOOK_BUILDER_H

#include "hooks/impls/NotificationHook.h"

const char * const _NOTIFICATION_HOOK_BUILDER_TAG = "notification_cb_builder";

class NotificationHookBuilder {
  public:
    template <typename T>
    static Hook<T>* build(JsonDocument doc) {
      return build<T>(static_cast<NotificationType>(doc[_nftHookField].as<int>()), doc[_messageHookField]);
    }

    template <typename T>
    static Hook<T> * build(const char * data) {
      return build<T>(static_cast<NotificationType>(data[0] - '0'), data + 1);
    }

// todo pass gateway url
    template <typename T>
    static Hook<T> * build(NotificationType type, String message) {
      if (type < NOTIFICATION_INFO || type > NOTIFICATION_ERROR) {
        st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Unknown notification type! (type=%d)", type);
        return nullptr;
      }

      if (message.isEmpty()) {
        st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Notification message is missing!");
        return nullptr;
      }

      message.replace("|;", ";");

      st_log_debug(
        _NOTIFICATION_HOOK_BUILDER_TAG,
        "Notification hook data:type=%s,message=%s",
        notificationTypeToStr(type),
        message.c_str()
      );

      return new NotificationHook<T>(type, message.c_str());
    }
};

#endif