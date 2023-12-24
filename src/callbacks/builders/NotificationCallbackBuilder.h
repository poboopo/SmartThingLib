#ifndef NOTIFICATION_CALLBACK_BUILDER_H
#define NOTIFICATION_CALLBACK_BUILDER_H

#include "CallbackBuilder.h"
#include "callbacks/impls/NotificationCallback.h"

#define NOTIFICATION_CALLBACK_BUILDER_TAG "notification_cb_builder"

namespace Callback {
class NotificationCallbackBuilder: public CallbackBuilder {
  public:
    template <typename T>
    static Callback<T>* build(JsonObject doc, bool readOnly) {
      if (doc.size() == 0) {
        LOGGER.error(NOTIFICATION_CALLBACK_BUILDER_TAG, "Json document is empty!");
        return nullptr;
      }
      const char * message = doc[MESSAGE_FIELD];
      if (message == nullptr || strlen(message) == 0) {
        LOGGER.error(NOTIFICATION_CALLBACK_BUILDER_TAG, "Message can't be empty");
        return nullptr;
      }

      NotificationCallback<T> * callback = new NotificationCallback<T>(message, readOnly);
      const char * type = doc[NOTIFICATION_TYPE_FIELD];
      if (type != nullptr && strlen(type) > 0) {
        callback->setNotificationType(type);
      }
      LOGGER.debug(
        NOTIFICATION_CALLBACK_BUILDER_TAG,
        "Notification callback created: type=%s, message=%s",
        callback->getNoticationType().c_str(),
        message
      );

      ::Callback::CallbackBuilder::defaultValues(callback, doc);

      return callback;
    }
    static DynamicJsonDocument getTemplate() {
      DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
      JsonObject msg = doc.createNestedObject(MESSAGE_FIELD);
      msg["required"] = true;
      
      JsonObject type = doc.createNestedObject(NOTIFICATION_TYPE_FIELD);
      JsonArray ar = type.createNestedArray("values");
      ar.add(NOTIFICATION_INFO);
      ar.add(NOTIFICATION_WARNING);
      ar.add(NOTIFICATION_ALERT);

      return doc;
    }
};
}

#endif