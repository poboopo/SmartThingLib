#ifndef ACTION_HOOK_BUILDER_H
#define ACTION_HOOK_BUILDER_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include "hooks/impls/ActionHook.h"
#include "logs/BetterLogger.h"

#define ACTION_HOOK_BUILDER_TAG "action_cb_builder"

namespace Hook {
class ActionHookBuilder {
 public:
  template <class B, typename T>
  static Hook<T>* build(JsonObject doc, bool readOnly) {
    if (doc.size() == 0) {
      LOGGER.error(ACTION_HOOK_BUILDER_TAG, "Json document is empty!");
      return nullptr;
    }
    const char* action = doc["action"];
    if (action == nullptr || strlen(action) == 0) {
      LOGGER.error(ACTION_HOOK_BUILDER_TAG, "Action can't be blank!");
      return nullptr;
    }

    ActionHook<B, T>* hook = new ActionHook<B, T>(action, readOnly);
    LOGGER.debug(ACTION_HOOK_BUILDER_TAG,
                 "Action hook created: action=%s", action);

    return hook;
  }
  static DynamicJsonDocument getTemplate() {
    DynamicJsonDocument doc(MAX_HOOK_TEMPLATE_SIZE);
    JsonObject actionObj = doc.createNestedObject("action");
    actionObj["required"] = true;
    actionObj["values"] = SmartThing.getActionsInfo();
    return doc;
  }
};
}  // namespace Hook
#endif

#endif