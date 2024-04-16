#ifndef HOOKS_FACTORY_H
#define HOOKS_FACTORY_H

#include <ArduinoJson.h>

#include "hooks/builders/ActionHookBuilder.h"
#include "hooks/builders/HttpHookBuilder.h"
#include "hooks/builders/NotificationHookBuilder.h"
#include "hooks/impls/ActionHook.h"
#include "hooks/impls/Hook.h"
#include "hooks/impls/HttpHook.h"

#define HOOKS_FACTORY_TAG "hooks_factory"

namespace Hook {
class HooksFactory {
 public:
  template <typename T>
  static Hook<T>* build(JsonObject doc) {
    const char* type = doc["type"];
    if (type == nullptr) {
      LOGGER.error(HOOKS_FACTORY_TAG, "Hook type is missing!");
      return nullptr;
    }

    LOGGER.debug(HOOKS_FACTORY_TAG,
                 "----------------------BUILDER-START----------------------");
    LOGGER.debug(HOOKS_FACTORY_TAG, "Building hook type=%s", type);

    Hook<T>* hook = nullptr;
    if (strcmp(type, ACTION_HOOK_TAG) == 0) {
      hook = ActionHookBuilder::build<T>(doc, false);
    } else if (strcmp(type, HTTP_HOOK_TAG) == 0) {
      hook = HttpHookBuilder::build<T>(doc, false);
    } else if (strcmp(type, NOTIFICATION_HOOK_TAG) == 0) {
      hook = NotificationHookBuilder::build<T>(doc, false);
    } else {
      LOGGER.error(HOOKS_FACTORY_TAG, "Unkonwn hook type: %s", type);
    }
    if (hook == nullptr) {
      LOGGER.debug(HOOKS_FACTORY_TAG, "Build failed");
    }

    LOGGER.debug(HOOKS_FACTORY_TAG,
                 "-----------------------BUILDER-END----------------------");
    return hook;
  }
  static DynamicJsonDocument getTemplates(const char * type) {
    DynamicJsonDocument doc(MAX_HOOK_TEMPLATE_SIZE * 4);
    doc["default"] = HookBuilder::getTemplate(type);
    doc[HTTP_HOOK_TAG] = HttpHookBuilder::getTemplate();
    doc[ACTION_HOOK_TAG] = ActionHookBuilder::getTemplate();
    doc[NOTIFICATION_HOOK_TAG] = NotificationHookBuilder::getTemplate();
    return doc;
  }
};
}  // namespace Hook

#endif