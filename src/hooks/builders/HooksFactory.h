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

#define CB_BUILDER_TRIGGER "trigger"
#define CB_BUILDER_COMPARE "compareType"

#define DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON                                  \
  "{\"threshold\":{\"required\":false},\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\",\"gte\",\"lte\"],\"default\":\"eq\"}}"
#define DEFAULT_STATES_HOOKS_TEMPLATES_JSON                                  \
  "{\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\"],\"default\":\"eq\"}}"

namespace Hook {
  class HooksFactory {
    public:
      // B - base class for hook (SensorHook/StateHook)
      template <class B, typename T>
      static Hook<T>* build(JsonObject doc) {
        const char* type = doc["type"];
        if (type == nullptr) {
          LOGGER.error(HOOKS_FACTORY_TAG, "Hook type is missing!");
          return nullptr;
        }

        LOGGER.debug(HOOKS_FACTORY_TAG,
                    "-----------------------BUILD-START-----------------------");
        LOGGER.debug(HOOKS_FACTORY_TAG, "Building hook type=%s", type);

        Hook<T>* hook = nullptr;
        if (strcmp(type, ACTION_HOOK_TAG) == 0) {
          hook = ActionHookBuilder::build<B, T>(doc, false);
        } else if (strcmp(type, HTTP_HOOK_TAG) == 0) {
          hook = HttpHookBuilder::build<B, T>(doc, false);
        } else if (strcmp(type, NOTIFICATION_HOOK_TAG) == 0) {
          hook = NotificationHookBuilder::build<B, T>(doc, false);
        } else {
          LOGGER.error(HOOKS_FACTORY_TAG, "Unkonwn hook type: %s", type);
        }
        if (hook == nullptr) {

        LOGGER.debug(HOOKS_FACTORY_TAG,
                    "-----------------------BUILD-FAILED---------------------");
          return nullptr;
        }

        if (doc.containsKey("id") && doc["id"].is<int>()) {
          uint8_t id = doc["id"];
          hook->setId(id);
          LOGGER.debug(HOOKS_FACTORY_TAG, "Id=%u", id);
        } else {
          hook->setId(-1);
          LOGGER.debug(HOOKS_FACTORY_TAG, "Id is empty");
        }

        String trigger = doc[CB_BUILDER_TRIGGER];
        if (trigger.isEmpty()) {
          hook->disableTrigger();
          LOGGER.debug(HOOKS_FACTORY_TAG, "Trigger disabled");
        } else {
          hook->enableTrigger();
          hook->setTriggerValue(doc[CB_BUILDER_TRIGGER].as<T>());
          
          LOGGER.debug(HOOKS_FACTORY_TAG, "Trigger=%s", trigger.c_str());
        }

        String compare = doc[CB_BUILDER_COMPARE];
        if (!compare.isEmpty()) {
          hook->setCompareType(compare);
          LOGGER.debug(HOOKS_FACTORY_TAG, "compareType=%s", compare.c_str());
        }

        setTypeSpecificValues(hook, doc);

        LOGGER.debug(HOOKS_FACTORY_TAG,
                    "------------------------BUILD-END-----------------------");
        return hook;
      }

      static DynamicJsonDocument getTemplates(const char * type) {
        DynamicJsonDocument doc(MAX_HOOK_TEMPLATE_SIZE * 4);
        doc["default"] = getDefaultTemplate(type);
        doc[HTTP_HOOK_TAG] = HttpHookBuilder::getTemplate();
        doc[ACTION_HOOK_TAG] = ActionHookBuilder::getTemplate();
        doc[NOTIFICATION_HOOK_TAG] = NotificationHookBuilder::getTemplate();
        return doc;
      }
    
    private:
      static DynamicJsonDocument getDefaultTemplate(const char * type) {
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
      static void setTypeSpecificValues(Hook<int16_t> * hook, JsonObject doc) {
        if (doc.containsKey("threshold")) {
          int16_t threshold = doc["threshold"];
          ((SensorHook *) hook)->setThreshold(threshold);
          LOGGER.debug(HOOKS_FACTORY_TAG, "Threshold=%d", threshold);
        }
      }
      static void setTypeSpecificValues(Hook<String> * hook, JsonObject doc) {
      }
  };
}  // namespace Hook

#endif