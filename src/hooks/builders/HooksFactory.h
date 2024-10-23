#ifndef HOOKS_FACTORY_H
#define HOOKS_FACTORY_H

#include <ArduinoJson.h>

#include "Features.h"
#include "hooks/builders/ActionHookBuilder.h"
#include "hooks/builders/HttpHookBuilder.h"
#include "hooks/builders/NotificationHookBuilder.h"
#include "hooks/impls/ActionHook.h"
#include "hooks/impls/Hook.h"
#include "hooks/impls/HttpHook.h"

#define CB_BUILDER_TRIGGER "trigger"
#define CB_BUILDER_COMPARE "compareType"

#define DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON                                  \
  "{\"threshold\":{\"required\":false},\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\",\"gte\",\"lte\"],\"default\":\"eq\"}}"
#define DEFAULT_STATES_HOOKS_TEMPLATES_JSON                                  \
  "{\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\"],\"default\":\"eq\"}}"

static const char * HOOKS_FACTORY_TAG = "hooks_factory";

namespace Hook {
  class HooksFactory {
    public:
      // B - base class for hook (SensorHook/StateHook)
      template <class B, typename T>
      static Hook<T>* build(JsonObject doc) {
        const char* type = doc["type"];
        if (type == nullptr) {
          SMT_LOG_ERROR(HOOKS_FACTORY_TAG, "Hook type is missing!");
          return nullptr;
        }

        SMT_LOG_DEBUG(HOOKS_FACTORY_TAG,
                    "-----------------------BUILD-START-----------------------");
        SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "Building hook type=%s", type);

        Hook<T>* hook = nullptr;
        #if ENABLE_ACTIONS 
        if (strcmp(type, ACTION_HOOK_TAG) == 0) {
          hook = ActionHookBuilder::build<B, T>(doc, false);
        } else 
        #endif
        if (strcmp(type, HTTP_HOOK_TAG) == 0) {
          hook = HttpHookBuilder::build<B, T>(doc, false);
        } else if (strcmp(type, NOTIFICATION_HOOK_TAG) == 0) {
          hook = NotificationHookBuilder::build<B, T>(doc, false);
        } else {
          SMT_LOG_ERROR(HOOKS_FACTORY_TAG, "Unkonwn hook type: %s", type);
        }
        if (hook == nullptr) {

        SMT_LOG_DEBUG(HOOKS_FACTORY_TAG,
                    "-----------------------BUILD-FAILED---------------------");
          return nullptr;
        }

        if (doc.containsKey("id") && doc["id"].is<int>()) {
          uint8_t id = doc["id"];
          hook->setId(id);
          SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "Id=%u", id);
        } else {
          hook->setId(-1);
          SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "Id is empty");
        }

        String trigger = doc[CB_BUILDER_TRIGGER];
        if (trigger.isEmpty() || trigger.equals("null")) {
          hook->disableTrigger();
          SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "Trigger disabled");
        } else {
          hook->enableTrigger();
          hook->setTriggerValue(doc[CB_BUILDER_TRIGGER].as<T>());
          
          SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "Trigger=%s", trigger.c_str());
        }

        String compare = doc[CB_BUILDER_COMPARE];
        if (!compare.isEmpty()) {
          hook->setCompareType(compare);
          SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "compareType=%s", compare.c_str());
        }

        setTypeSpecificValues(hook, doc);

        SMT_LOG_DEBUG(HOOKS_FACTORY_TAG,
                    "------------------------BUILD-END-----------------------");
        return hook;
      }

      static JsonDocument getTemplates(const char * type) {
        JsonDocument doc;
        doc["default"] = getDefaultTemplate(type);
        #if ENABLE_ACTIONS
        if (ActionsManager.count() > 0) {
          doc[ACTION_HOOK_TAG] = ActionHookBuilder::getTemplate();
        }
        #endif
        doc[HTTP_HOOK_TAG] = HttpHookBuilder::getTemplate();
        doc[NOTIFICATION_HOOK_TAG] = NotificationHookBuilder::getTemplate();
        return doc;
      }
    
    private:
      static JsonDocument getDefaultTemplate(const char * type) {
        JsonDocument doc;
        if (strcmp(type, SENSOR_TYPE) == 0) {
          deserializeJson(doc, DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON);
        } else if (strcmp(type, STATE_TYPE) == 0) {
          deserializeJson(doc, DEFAULT_STATES_HOOKS_TEMPLATES_JSON);
        } else {
          deserializeJson(doc, "{}");
        }
        return doc;
      }
      #if ENABLE_SENSORS
      static void setTypeSpecificValues(Hook<int16_t> * hook, JsonObject doc) {
        if (doc.containsKey("threshold")) {
          int16_t threshold = doc["threshold"];
          ((SensorHook *) hook)->setThreshold(threshold);
          SMT_LOG_DEBUG(HOOKS_FACTORY_TAG, "Threshold=%d", threshold);
        }
      }
      #endif
      static void setTypeSpecificValues(Hook<String> * hook, JsonObject doc) {
      }
  };
}  // namespace Hook

#endif