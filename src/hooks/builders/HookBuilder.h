#ifndef HOOK_BUILDER_H
#define HOOK_BUILDER_H

#include <ArduinoJson.h>

#include "hooks/impls/Hook.h"

#define CB_BUILDER_TRIGGER "trigger"
#define CB_BUILDER_COMPARE "compareType"

#define HOOK_BUILDER_TAG "cb_builder"

#define DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON                                  \
  "{\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\",\"gte\",\"lte\"],\"default\":\"eq\"}}"
#define DEFAULT_STATES_HOOKS_TEMPLATES_JSON                                  \
  "{\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\"],\"default\":\"eq\"}}"

namespace Hook {
class HookBuilder {
 public:
  template <typename T>
  static Hook<T>* build(JsonObject doc) {
    LOGGER.error(HOOK_BUILDER_TAG, "Base build method call!");
    return nullptr;
  };
  static DynamicJsonDocument getTemplate(const char * type) {
    DynamicJsonDocument doc(MAX_HOOK_TEMPLATE_SIZE);
    if (strcmp(type, SENSOR_TYPE) == 0) {
      deserializeJson(doc, DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON);
    } else if (strcmp(type, STATE_TYPE) == 0) {
      deserializeJson(doc, DEFAULT_STATES_HOOKS_TEMPLATES_JSON);
    } else {
      deserializeJson(doc, "{}");
    }
    return doc;
  }

 protected:
  template <typename T>
  static void defaultValues(Hook<T>* hook, JsonObject doc) {
    if (doc.containsKey("id") && doc["id"].is<int>()) {
      uint8_t id = doc["id"];
      hook->setId(id);
      LOGGER.debug(HOOK_BUILDER_TAG, "Id=%u", id);
    } else {
      hook->setId(-1);
      LOGGER.debug(HOOK_BUILDER_TAG, "Id is empty");
    }

    const char* trigger = doc[CB_BUILDER_TRIGGER];
    if (trigger == nullptr || strlen(trigger) == 0) {
      hook->setTriggerDisabled(true);

      LOGGER.debug(HOOK_BUILDER_TAG, "Trigger disabled");
    } else {
      hook->setTriggerDisabled(false);
      hook->setTriggerValue(doc[CB_BUILDER_TRIGGER].as<T>());
      String compare = doc[CB_BUILDER_COMPARE];
      hook->setCompareType(compare);

      LOGGER.debug(HOOK_BUILDER_TAG, "Trigger=%s, compareType=%s", trigger,
                   compare.c_str());
    }
  }
};
}  // namespace Hook

#endif