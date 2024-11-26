#ifndef HOOKS_FACTORY_H
#define HOOKS_FACTORY_H

#include <ArduinoJson.h>
#include <type_traits>

#include "Features.h"
#include "hooks/builders/ActionHookBuilder.h"
#include "hooks/builders/HttpHookBuilder.h"
#include "hooks/builders/NotificationHookBuilder.h"
#include "hooks/impls/ActionHook.h"
#include "hooks/impls/Hook.h"
#include "hooks/impls/HttpHook.h"

static const char * _HOOKS_BUILDER_TAG = "hooks_factory";

#define DEFAULT_SENSORS_HOOKS_TEMPLATES_JSON                                  \
  "{\"threshold\":{\"required\":false},\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\",\"gte\",\"lte\"],\"default\":\"eq\"}}"
#define DEFAULT_STATES_HOOKS_TEMPLATES_JSON                                  \
  "{\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\"],\"default\":\"eq\"}}"

class HooksBuilder {
  public:
    template <typename T>
    static Hook<T>* build(const char * data) { // todo pass data length
      if (data == nullptr || strlen(data) == 0) {
        st_log_error(_HOOKS_BUILDER_TAG, "Hook's string missing!");
        return nullptr;
      }

      bool isJson = false;
      if (data[0] == '{') {
        isJson = true;
      } else if (data[0] < '0' && data[0] > '9') {
        st_log_error(_HOOKS_BUILDER_TAG, "Bad input string - not json object or repo string format");
        return nullptr;
      } else if (data == nullptr || strlen(data) < 9) {
        st_log_error(_HOOKS_BUILDER_TAG, "Bad input string - min length 9");
        return nullptr;
      }

      st_log_debug(_HOOKS_BUILDER_TAG, "-----------------------BUILD-START-----------------------");

      JsonDocument doc;
      HookType type;
      int id = -1;
      CompareType compareType = UNKNOWN_COMPARE;
      bool triggerEnabled = false;
      String triggerString;
      int dataOffset = 0;

      if (isJson) {
        deserializeJson(doc, data);

        type = hookTypeFromStr(doc[_typeHookField]);
        compareType = compareTypeFromString(doc[_compareTypeHookField].as<const char*>());

        if (doc[_idHookField].is<int>()) {
          id = doc[_idHookField];
        } else {
          st_log_debug(_HOOKS_BUILDER_TAG, "Id is missing");
        }
        
        if (doc[_triggerEnabledHookField].is<JsonVariant>()) {
          triggerEnabled = doc[_triggerEnabledHookField].as<bool>();
        }

      } else {
        type = static_cast<HookType>(data[0] - '0');
        compareType = static_cast<CompareType>(data[3] - '0');
        triggerEnabled = data[4] == '1';

        char idStr[] = {data[1], data[2]};
        id = atoi(idStr);

        for (dataOffset = 5; dataOffset < strlen(data); dataOffset++) {
          if (data[dataOffset] == ';') {
            break;
          }
          triggerString += data[dataOffset];
        }
        dataOffset++;
      }
      
      st_log_debug(
        _HOOKS_BUILDER_TAG,
        "Base hook data: type=%s, id=%d, compare_type=%d, trigger_enabled=%d",
        hookTypeToStr(type), id, compareType, triggerEnabled
      );
      
      if (type == UNKNOWN_HOOK) {
        st_log_error(_HOOKS_BUILDER_TAG, "Unknown hook type!");
        return nullptr;
      }

      Hook<T> * hook = nullptr;
      if (isJson) {
        hook = buildForType<T, JsonDocument>(type, doc);
      } else {
        hook = buildForType<T, const char *>(type, data + dataOffset);
      }

      if (hook == nullptr) {
        st_log_debug(_HOOKS_BUILDER_TAG,
                  "-----------------------BUILD-FAILED---------------------");
        return nullptr;
      }

      hook->setId(id);
      hook->setCompareType(compareType);
      hook->setTriggerEnabled(triggerEnabled);
      hook->setReadOnly(false);

      if (isJson) {
        parseTrigger(hook, doc);
      } else {
        parseTrigger(hook, triggerString);
      }
      
      st_log_debug(_HOOKS_BUILDER_TAG,
                  "----------------------BUILD-SUCCESS---------------------");
      return hook;
    }

    static String getTemplates(ObservableType type) {
      JsonDocument doc;
      doc["default"] = getDefaultTemplate(type);
      #if ENABLE_ACTIONS
      if (ActionsManager.count() > 0) {
        doc[_actionHookType] = ActionHookBuilder::getTemplate();
      }
      #endif
      doc[_httpHookType] = HttpHookBuilder::getTemplate();
      doc[_notificationHookType] = NotificationHookBuilder::getTemplate();

      String result;
      serializeJson(doc, result);
      
      return result;
    }
  
    #if ENABLE_SENSORS
    static void parseTrigger(Hook<SENSOR_DATA_TYPE> * hook, JsonDocument &doc) {
      st_log_debug(_HOOKS_BUILDER_TAG, "trigger=%d", doc[_triggerHookField].as<int>());
      st_log_debug(_HOOKS_BUILDER_TAG, "threshold=%d", doc[_thresholdHookField].as<int>());
      hook->setTriggerValue(doc[_triggerHookField]);
      ((SensorHook *) hook)->setThreshold(doc[_thresholdHookField]);
    }
    #endif

    #if ENABLE_STATES
    static void parseTrigger(Hook<STATE_DATA_TYPE> * hook, JsonDocument &doc) {
      st_log_debug(_HOOKS_BUILDER_TAG, "trigger=%s", doc[_triggerHookField].as<String>().c_str());
      hook->setTriggerValue(doc[_triggerHookField]);
    }
    #endif

  private:
    template<typename T, typename D>
    static Hook<T> * buildForType(HookType type, D data) {
      switch (type) {
        #if ENABLE_ACTIONS 
        case ACTION_HOOK:
          return ActionHookBuilder::build<T>(data);
        #endif
        case HTTP_HOOK:
          return HttpHookBuilder::build<T>(data);
        case NOTIFICATION_HOOK:
          return NotificationHookBuilder::build<T>(data);
        default:
          st_log_error(_HOOKS_BUILDER_TAG, "Hook of type %u not supported", type);
      }
      return nullptr;
    }

    #if ENABLE_SENSORS
    static void parseTrigger(Hook<SENSOR_DATA_TYPE> * hook, String trigger) {
      String buff;
      int tmp;

      for (int i = 0; i < trigger.length(); i++) {
        if (trigger.charAt(i) == '_') {
          tmp = buff.toInt();
          st_log_debug(_HOOKS_BUILDER_TAG, "trigger=%d", tmp);
          hook->setTriggerValue(tmp);
          buff.clear();
        } else {
          buff += trigger.charAt(i);
        }
      }

      tmp = buff.toInt();
      st_log_debug(_HOOKS_BUILDER_TAG, "threshold=%d", tmp);
      ((SensorHook *) hook)->setThreshold(tmp);
    }
    #endif

    #if ENABLE_STATES
    static void parseTrigger(Hook<STATE_DATA_TYPE> * hook, String &trigger) {
      trigger.replace("|;", ";");
      st_log_debug(_HOOKS_BUILDER_TAG, "trigger=%s", trigger.c_str());
      hook->setTriggerValue(trigger.c_str());
    }
    #endif

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
};

#endif