#ifndef ACTION_HOOK_BUILDER_H
#define ACTION_HOOK_BUILDER_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include "hooks/impls/ActionHook.h"
#include "logs/BetterLogger.h"
#include "actions/ActionsManager.h"

static const char * ACTION_HOOK_BUILDER_TAG = "action_cb_builder";

namespace Hook {
class ActionHookBuilder {
 public:
  template <class B, typename T>
  static Hook<T>* build(JsonObject doc, bool readOnly) {
    if (doc.size() == 0) {
      SMT_LOG_ERROR(ACTION_HOOK_BUILDER_TAG, "Json document is empty!");
      return nullptr;
    }
    const char* action = doc["action"];
    if (action == nullptr || strlen(action) == 0) {
      SMT_LOG_ERROR(ACTION_HOOK_BUILDER_TAG, "Action can't be blank!");
      return nullptr;
    }

    ActionHook<B, T>* hook = new ActionHook<B, T>(action, readOnly);
    SMT_LOG_DEBUG(ACTION_HOOK_BUILDER_TAG,
                 "Action hook created: action=%s", action);

    return hook;
  }
  static JsonDocument getTemplate() {
    JsonDocument doc;
    JsonObject actionObj = doc["action"].to<JsonObject>();
    actionObj["required"] = true;
    actionObj["values"] = ActionsManager.toJson();
    return doc;
  }
};
}  // namespace Hook
#endif

#endif