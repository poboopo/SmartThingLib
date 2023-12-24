#ifndef CALLBACK_BUILDER_H
#define CALLBACK_BUILDER_H

#include <ArduinoJson.h>

#include "callbacks/impls/Callback.h"

#define CB_BUILDER_TRIGGER "trigger"
#define CB_BUILDER_COMPARE "compareType"

#define CALLBACK_BUILDER_TAG "cb_builder"

#define DEFAULT_CALLBACKS_TEMPLATES_JSON                                  \
  "{\"trigger\":{\"required\":false},\"compareType\":{\"required\":true," \
  "\"values\":[\"eq\",\"neq\",\"gte\",\"lte\"],\"default\":\"eq\"}}"

namespace Callback {
class CallbackBuilder {
 public:
  template <typename T>
  static Callback<T>* build(JsonObject doc) {
    LOGGER.error(CALLBACK_BUILDER_TAG, "Base build method call!");
    return nullptr;
  };
  static DynamicJsonDocument getTemplate() {
    DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
    deserializeJson(doc, DEFAULT_CALLBACKS_TEMPLATES_JSON);
    return doc;
  }

 protected:
  template <typename T>
  static void defaultValues(Callback<T>* callback, JsonObject doc) {
    if (doc.containsKey("id") && doc["id"].is<int>()) {
      uint8_t id = doc["id"];
      callback->setId(id);
      LOGGER.debug(CALLBACK_BUILDER_TAG, "Id=%u", id);
    } else {
      callback->setId(-1);
      LOGGER.debug(CALLBACK_BUILDER_TAG, "Id is empty");
    }

    const char* trigger = doc[CB_BUILDER_TRIGGER];
    if (trigger == nullptr || strlen(trigger) == 0) {
      callback->setTriggerDisabled(true);

      LOGGER.debug(CALLBACK_BUILDER_TAG, "Trigger disabled");
    } else {
      callback->setTriggerDisabled(false);
      callback->setTriggerValue(doc[CB_BUILDER_TRIGGER].as<T>());
      String compare = doc[CB_BUILDER_COMPARE];
      callback->setCompareType(compare);

      LOGGER.debug(CALLBACK_BUILDER_TAG, "Trigger=%s, compareType=%s", trigger,
                   compare.c_str());
    }
  }
};
}  // namespace Callback

#endif