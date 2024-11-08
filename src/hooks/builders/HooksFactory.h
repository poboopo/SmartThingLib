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

class HooksFactory {
  public:
    // B - base class for hook (SensorHook/StateHook)
    template <class B, typename T>
    static Hook<T>* build(JsonObject doc) {
      const char* type = doc["type"];
      if (type == nullptr) {
        st_log_error(HOOKS_FACTORY_TAG, "Hook type is missing!");
        return nullptr;
      }

      st_log_debug(HOOKS_FACTORY_TAG,
                  "-----------------------BUILD-START-----------------------");
      st_log_debug(HOOKS_FACTORY_TAG, "Building hook type=%s", type);

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
        st_log_error(HOOKS_FACTORY_TAG, "Unkonwn hook type: %s", type);
      }
      if (hook == nullptr) {
        st_log_debug(HOOKS_FACTORY_TAG,
                  "-----------------------BUILD-FAILED---------------------");
        return nullptr;
      }

      if (doc.containsKey("id") && doc["id"].is<int>()) {
        uint8_t id = doc["id"];
        hook->setId(id);
        st_log_debug(HOOKS_FACTORY_TAG, "Id=%u", id);
      } else {
        hook->setId(-1);
        st_log_debug(HOOKS_FACTORY_TAG, "Id is missing");
      }

      update<T>(hook, doc);

      st_log_debug(HOOKS_FACTORY_TAG,
                  "------------------------BUILD-END-----------------------");
      return hook;
    }

    template <typename T>
    static void update(Hook<T> * hook, JsonObject doc) {
      if (doc.containsKey("triggerEnabled")) {
        if (doc["triggerEnabled"].as<bool>()) {
          hook->enableTrigger();
          st_log_debug(HOOKS_FACTORY_TAG, "Trigger enabled");
        } else {
          hook->disableTrigger();
          st_log_debug(HOOKS_FACTORY_TAG, "Trigger disabled");
        }
      }

      hook->setTriggerValue(doc[CB_BUILDER_TRIGGER]);
      st_log_debug(HOOKS_FACTORY_TAG, "Trigger=%s", doc[CB_BUILDER_TRIGGER].as<String>().c_str());

      hook->setCompareType(doc[CB_BUILDER_COMPARE].as<const char*>());
      st_log_debug(
        HOOKS_FACTORY_TAG,
        "compareType=%s",
        compareTypeToString(hook->getCompareType()).c_str()
      );

      setTypeSpecificValues(hook, doc);
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
        st_log_debug(HOOKS_FACTORY_TAG, "Threshold=%d", threshold);
      }
    }
    #endif
    static void setTypeSpecificValues(Hook<String> * hook, JsonObject doc) {
    }
};

#endif