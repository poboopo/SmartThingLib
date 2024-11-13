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

static const char * _HOOKS_FACTORY_TAG = "hooks_factory";

class HooksFactory {
  public:
    // B - base class for hook (SensorHook/StateHook)
    template <class B, typename T>
    static Hook<T>* build(JsonObject doc) {
      HookType type = hookTypeFromStr(doc["type"]);
      if (type == UNKNOWN_HOOK) {
        st_log_error(_HOOKS_FACTORY_TAG, "Can't select hook type!");
        return nullptr;
      }

      st_log_debug(_HOOKS_FACTORY_TAG,
                  "-----------------------BUILD-START-----------------------");
      st_log_debug(_HOOKS_FACTORY_TAG, "Building hook type=%u", type);

      Hook<T>* hook = nullptr;
      switch (type) {
        #if ENABLE_ACTIONS 
        case ACTION_HOOK:
          hook = ActionHookBuilder::build<B, T>(doc, false);
          break;
        #endif
        case HTTP_HOOK:
          hook = HttpHookBuilder::build<B, T>(doc, false);
          break;
        case NOTIFICATION_HOOK:
          hook = NotificationHookBuilder::build<B, T>(doc, false);
          break;
        default:
          st_log_error(_HOOKS_FACTORY_TAG, "Hook of type %u not supported", type);
      }

      if (hook == nullptr) {
        st_log_debug(_HOOKS_FACTORY_TAG,
                  "-----------------------BUILD-FAILED---------------------");
        return nullptr;
      }

      if (doc["id"].is<int>()) {
        uint8_t id = doc["id"];
        hook->setId(id);
        st_log_debug(_HOOKS_FACTORY_TAG, "Id=%u", id);
      } else {
        hook->setId(-1);
        st_log_debug(_HOOKS_FACTORY_TAG, "Id is missing");
      }

      update<T>(hook, doc);

      st_log_debug(_HOOKS_FACTORY_TAG,
                  "------------------------BUILD-END-----------------------");
      return hook;
    }

    template <typename T>
    static void update(Hook<T> * hook, JsonObject doc) {
      if (doc["triggerEnabled"].is<bool>()) {
        if (doc["triggerEnabled"].as<bool>()) {
          hook->enableTrigger();
          st_log_debug(_HOOKS_FACTORY_TAG, "Trigger enabled");
        } else {
          hook->disableTrigger();
          st_log_debug(_HOOKS_FACTORY_TAG, "Trigger disabled");
        }
      }

      hook->setTriggerValue(doc[CB_BUILDER_TRIGGER]);
      st_log_debug(_HOOKS_FACTORY_TAG, "Trigger=%s", doc[CB_BUILDER_TRIGGER].as<String>().c_str());

      hook->setCompareType(doc[CB_BUILDER_COMPARE].as<const char*>());
      st_log_debug(
        _HOOKS_FACTORY_TAG,
        "compareType=%s",
        compareTypeToString(hook->getCompareType()).c_str()
      );

      setTypeSpecificValues(hook, doc);
    }

    static JsonDocument getTemplates(ObservableType type) {
      JsonDocument doc;
      doc["default"] = getDefaultTemplate(type);
      #if ENABLE_ACTIONS
      if (ActionsManager.count() > 0) {
        doc[_actionHookStr] = ActionHookBuilder::getTemplate();
      }
      #endif
      doc[_httpHookStr] = HttpHookBuilder::getTemplate();
      doc[_lambdaHookStr] = NotificationHookBuilder::getTemplate();
      return doc;
    }
  
  private:
    static JsonDocument getDefaultTemplate(ObservableType type) {
      JsonDocument doc;
      switch (type) {
        case OBS_SENSOR:
          deserializeJson(doc, DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON);
          break;
        case OBS_STATE:
          deserializeJson(doc, DEFAULT_STATES_HOOKS_TEMPLATES_JSON);
          break;
        default:
          deserializeJson(doc, "{}");
      }
      return doc;
    }
    #if ENABLE_SENSORS
    static void setTypeSpecificValues(Hook<int16_t> * hook, JsonObject doc) {
      if (doc["threshold"].is<int>()) {
        int threshold = doc["threshold"];
        ((SensorHook *) hook)->setThreshold(threshold);
        st_log_debug(_HOOKS_FACTORY_TAG, "Threshold=%d", threshold);
      }
    }
    #endif
    static void setTypeSpecificValues(Hook<String> * hook, JsonObject doc) {
    }
};

#endif