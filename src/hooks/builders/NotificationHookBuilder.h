#ifndef NOTIFICATION_HOOK_BUILDER_H
#define NOTIFICATION_HOOK_BUILDER_H

#include "hooks/impls/NotificationHook.h"

const char * const _NOTIFICATION_HOOK_BUILDER_TAG = "notification_cb_builder";

class NotificationHookBuilder {
  public:
    template <typename T>
    static Hook<T>* build(JsonDocument doc) {
      return build<T>(
        static_cast<NotificationType>(doc[_nftHookField].as<int>()),
        doc[_messageHookField]
        #if !(ENABLE_CONFIG)
          ,doc[_gatewayHookField]
        #endif
      );
    }

    template <typename T>
    static Hook<T> * build(const char * data) {
      #if ENABLE_CONFIG
        return build<T>(
          static_cast<NotificationType>(data[0] - '0'),
          data + 1
        );
      #else
        int index = -1;
        bool escaped = false;
        for (int i = 1; i < strlen(data); i++) {
          if (escaped) {
            escaped = false;
          } else if (data[i] == '|') {
            escaped = true;
          } else if (data[i] == ';') {
            index = i;
            break;
          }
        }

        if (escaped == -1) {
          st_log_error(_NOTIFICATION_HOOK_TAG, "Bad input data - can't find ;");
          return nullptr;
        }

        String tmp = data;
        return build<T>(
          static_cast<NotificationType>(data[0] - '0'),
          tmp.substring(1, index),
          tmp.substring(index + 1)
        );
      #endif
    }

    template <typename T>
    static Hook<T> * build(
      NotificationType type,
      String message
      #if !(ENABLE_CONFIG)
        ,String gatewayUrl
      #endif
    ) {
      if (type < NOTIFICATION_INFO || type > NOTIFICATION_ERROR) {
        st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Unknown notification type! (type=%d)", type);
        return nullptr;
      }

      if (message.isEmpty()) {
        st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Notification message is missing!");
        return nullptr;
      }
      message.replace("|;", ";");

      #if !(ENABLE_CONFIG)
        if (gatewayUrl.isEmpty()) {
          st_log_error(_NOTIFICATION_HOOK_BUILDER_TAG, "Notification message is missing!");
          return nullptr;
        }
        gatewayUrl.replace("|;", ";");

        st_log_debug(
          _NOTIFICATION_HOOK_BUILDER_TAG,
          "Notification hook data:type=%s,message=%s,gatewayUrl=%s",
          notificationTypeToStr(type),
          message.c_str(),
          gatewayUrl.c_str()
        );
        return new NotificationHook<T>(type, message.c_str(), gatewayUrl.c_str());
      #else
        st_log_debug(
          _NOTIFICATION_HOOK_BUILDER_TAG,
          "Notification hook data:type=%s,message=%s",
          notificationTypeToStr(type),
          message.c_str()
        );
        return new NotificationHook<T>(type, message.c_str());
      #endif
    }
};

#endif