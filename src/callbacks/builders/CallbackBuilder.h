#ifndef CALLBACK_BUILDER_H
#define CALLBACK_BUILDER_H

#include <ArduinoJson.h>
#include "callbacks/impls/Callback.h"

#define CB_BUILDER_TRIGGER "trigger"
#define CB_BUILDER_COMPARE "compareType"

#define CALLBACK_BUILDER_TAG "cb_builder"

namespace Callback {
  template<typename T>
  class CallbackBuilder {
    public:
      static Callback<T> * build(JsonObject doc) {
        LOGGER.error(CALLBACK_BUILDER_TAG, "Base build method call!");
        return nullptr;
      };
    protected:
      static void defaultValues(Callback<T> * callback, JsonObject doc) {
        const char * id = doc["id"];
        if (id != nullptr && strlen(id) != 0) {
          callback->setId(atoi(id));
          LOGGER.debug(CALLBACK_BUILDER_TAG, "Callback id=%s", id);
        }

        const char * trigger = doc[CB_BUILDER_TRIGGER];
        if (trigger == nullptr || strlen(trigger) == 0) {
          callback->setTriggerDisabled(true);

          LOGGER.debug(CALLBACK_BUILDER_TAG, "Trigger disabled");
        } else {
          callback->setTriggerDisabled(false);
          callback->setTriggerValue(doc[CB_BUILDER_TRIGGER].as<T>());
          String compare = doc[CB_BUILDER_COMPARE];
          callback->setCompareType(compare);

          LOGGER.debug(CALLBACK_BUILDER_TAG, "Trigger=%s, compareType=%s", trigger, compare.c_str());
        }
      }
  };
}

#endif