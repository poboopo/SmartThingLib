#ifndef ACTION_CALLBACK_BUILDER_H
#define ACTION_CALLBACK_BUILDER_H

#include "CallbackBuilder.h"
#include "callbacks/impls/ActionCallback.h"
#include "logs/BetterLogger.h"

#define ACTION_CALLBACK_BUILDER_TAG "action_cb_builder"

namespace Callback {
class ActionCallbackBuilder : public CallbackBuilder {
 public:
  template <typename T>
  static Callback<T>* build(JsonObject doc, bool readOnly) {
    if (doc.size() == 0) {
      LOGGER.error(ACTION_CALLBACK_BUILDER_TAG, "Json document is empty!");
      return nullptr;
    }
    const char* action = doc["action"];
    if (action == nullptr || strlen(action) == 0) {
      LOGGER.error(ACTION_CALLBACK_BUILDER_TAG, "Action can't be blank!");
      return nullptr;
    }

    ActionCallback<T>* callback = new ActionCallback<T>(action, readOnly);
    LOGGER.debug(ACTION_CALLBACK_BUILDER_TAG,
                 "Action callback created: action=%s", action);

    ::Callback::CallbackBuilder::defaultValues(callback, doc);

    return callback;
  }
  static DynamicJsonDocument getTemplate() {
    DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
    JsonObject actionObj = doc.createNestedObject("action");
    actionObj["required"] = true;
    actionObj["values"] = SmartThing.getActionsInfo();
    return doc;
  }
};
}  // namespace Callback

#endif