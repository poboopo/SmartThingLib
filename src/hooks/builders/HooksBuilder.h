#ifndef HOOKS_BUILDER_H
#define HOOKS_BUILDER_H

#include <type_traits>

#include "Features.h"
#include "hooks/builders/ActionHookBuilder.h"
#include "hooks/builders/HttpHookBuilder.h"
#include "hooks/builders/NotificationHookBuilder.h"
#include "hooks/impls/ActionHook.h"
#include "hooks/impls/Hook.h"
#include "hooks/impls/HttpHook.h"

const char * const _HOOKS_BUILDER_TAG = "hooks_factory";

#if ENABLE_ACTIONS
  const char * const TEMPLATES_JSON = "{\"default\":%s,\"%s\":%s,\"%s\":%s,\"%s\":%s}";
  const char * const ACTION_HOOK_TEMPLATE = "{\"action\":{\"required\":true,\"values\":{%s}}}";
  const size_t TEMPLATE_JSON_LENGTH = 24;
  const size_t ACTION_HOOK_TEMPLATE_LENGTH = 42;
#else
  const char * const TEMPLATES_JSON = "{\"default\":%s,\"%s\":%s,\"%s\":%s}";
  const size_t TEMPLATE_JSON_LENGTH = 20;
#endif

// todo merge common templates
const char * const DEFAULT_NUMBER_HOOKS_TEMPLATES_JSON = "{\"triggerEnabled\":{\"required\":false,\"type\":\"checkbox\"},\"trigger\":{\"required\":false,\"type\":\"number\"},\"threshold\":{\"required\":false,\"type\":\"number\"},\"compareType\":{\"required\":true,\"values\":[\"eq\",\"neq\",\"gte\",\"lte\"],\"default\":\"eq\"}}";
const char * const DEFAULT_TEXT_HOOKS_TEMPLATES_JSON = "{\"triggerEnabled\":{\"required\":false,\"type\":\"checkbox\"},\"trigger\":{\"required\":false,\"type\":\"text\"},\"compareType\":{\"required\":true,\"values\":[\"eq\",\"neq\"],\"default\":\"eq\"}}";
const char * const HTTP_HOOK_TEMPLATE = "{\"url\":{\"required\":true},\"payload\":{\"required\":false},\"method\":{\"required\":true,\"values\":{\"1\":\"GET\",\"2\":\"POST\",\"3\":\"PUT\",\"4\":\"PATCH\",\"5\":\"DELETE\"}}}";
#if ENABLE_CONFIG
  const char * const NOTIFICATION_HOOK_TEMPLATE = "{\"message\":{\"required\":true},\"notificationType\":{\"values\":{\"1\":\"info\",\"2\":\"warning\",\"3\":\"error\"}}}";
#else
  const char * const NOTIFICATION_HOOK_TEMPLATE = "{\"gatewayUrl\":{\"required\":true},\"message\":{\"required\":true},\"notificationType\":{\"values\":{\"1\":\"info\",\"2\":\"warning\",\"3\":\"error\"}}}";
#endif

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

        for (dataOffset = 5; dataOffset < (int) strlen(data); dataOffset++) {
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

    static String getTemplates(const char * name) {
      SensorType type = SensorsManager.getSensorType(name);
      if (type == UNKNOWN_SENSOR) {
        return "{}";
      }

      const char * defTemp = getDefaultTemplate(type);

      size_t nameLen = strlen(_httpHookType) + strlen(_notificationHookType);
      size_t tempLen = strlen(defTemp) + strlen(HTTP_HOOK_TEMPLATE) + strlen(NOTIFICATION_HOOK_TEMPLATE);
      
      #if ENABLE_ACTIONS
        String actionsBuff = ActionsManager.getActionsInfoForHook();

        size_t actionTempLen = actionsBuff.length() + ACTION_HOOK_TEMPLATE_LENGTH;
        char actionTemp[actionTempLen + 1];
        sprintf(actionTemp, ACTION_HOOK_TEMPLATE, actionsBuff.c_str());

        nameLen += strlen(_actionHookType);
        tempLen += actionTempLen;
      #endif

      char buff[TEMPLATE_JSON_LENGTH + nameLen + tempLen + 1];

      #if ENABLE_ACTIONS
      sprintf(
        buff,
        TEMPLATES_JSON,
        defTemp,
        _actionHookType, actionTemp,
        _httpHookType, HTTP_HOOK_TEMPLATE,
        _notificationHookType, NOTIFICATION_HOOK_TEMPLATE
      );
      #else
      sprintf(
        buff,
        TEMPLATES_JSON,
        defTemp,
        _httpHookType, HTTP_HOOK_TEMPLATE,
        _notificationHookType, NOTIFICATION_HOOK_TEMPLATE
      );
      #endif

      String result = buff;
      return result;
    }
  
    #if ENABLE_NUMBER_SENSORS
    static void parseTrigger(Hook<NUMBER_SENSOR_DATA_TYPE> * hook, JsonDocument &doc) {
      st_log_debug(_HOOKS_BUILDER_TAG, "trigger=%d", doc[_triggerHookField].as<int>());
      st_log_debug(_HOOKS_BUILDER_TAG, "threshold=%d", doc[_thresholdHookField].as<int>());
      hook->setTriggerValue(doc[_triggerHookField]);
      ((NumberSensorHook *) hook)->setThreshold(doc[_thresholdHookField]);
    }
    #endif

    #if ENABLE_TEXT_SENSORS
    static void parseTrigger(Hook<TEXT_SENSOR_DATA_TYPE> * hook, JsonDocument &doc) {
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

    #if ENABLE_NUMBER_SENSORS
    static void parseTrigger(Hook<NUMBER_SENSOR_DATA_TYPE> * hook, String trigger) {
      String buff;
      int tmp;

      for (unsigned int i = 0; i < trigger.length(); i++) {
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
      ((NumberSensorHook *) hook)->setThreshold(tmp);
    }
    #endif

    #if ENABLE_TEXT_SENSORS
    static void parseTrigger(Hook<TEXT_SENSOR_DATA_TYPE> * hook, String &trigger) {
      trigger.replace("|;", ";");
      st_log_debug(_HOOKS_BUILDER_TAG, "trigger=%s", trigger.c_str());
      hook->setTriggerValue(trigger.c_str());
    }
    #endif

    static const char * getDefaultTemplate(SensorType type) {
      #if ENABLE_NUMBER_SENSORS
      if (type == NUMBER_SENSOR) {
        return DEFAULT_NUMBER_HOOKS_TEMPLATES_JSON;
      }
      #endif

      #if ENABLE_TEXT_SENSORS
      if (type == TEXT_SENSOR) {
        return DEFAULT_TEXT_HOOKS_TEMPLATES_JSON;
      }
      #endif

      return "{}";
    }
};

#endif