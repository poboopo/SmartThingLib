#ifndef WATCHER_CALLBACK_H
#define WATCHER_CALLBACK_H

#define WATCHER_CALLBACK_TAG "watcher_callback"

#include <ArduinoJson.h>

#include <functional>

#include "callbacks/comparator/Comparator.h"
#include "logs/BetterLogger.h"

#define CALLBACK_INFO_DOC_SIZE 512
#define MAX_CALLBACK_TEMPLATE_SIZE 1024

namespace Callback {
template <typename T>
// template<ConfigurableObject<T> ????>
class Callback {
 public:
  Callback(const char *type, bool readonly)
      : _id(-1),
        _type(type),
        _readonly(readonly),
        _compareType(EQ),
        _triggerDisabled(true){};

  // todo make value const
  virtual void call(T &value) = 0;
  virtual DynamicJsonDocument toJson(bool shortJson) = 0;
  virtual void updateCustom(JsonObject doc) = 0;

  bool accept(T &value) {
    return _triggerDisabled ||
           Comparator::compare(_compareType, value, _triggerValue);
  }

  void addDefaultInfo(DynamicJsonDocument &doc) {
    doc["id"] = _id;
    doc["readonly"] = _readonly;
    doc["type"] = _type;
    doc["trigger"] = _triggerValue;
    doc["compareType"] = compareTypeToString(_compareType);
  }

  void setId(int id) { _id = id; }
  const int getId() const { return _id; }
  void setCompareType(String type) {
    _compareType = compareTypeFromString(type, CompareType::EQ);
  }
  void setCompareType(CompareType type) { _compareType = type; }
  void setTriggerDisabled(bool disabled) { _triggerDisabled = disabled; }
  void setTriggerValue(T triggerValue) { _triggerValue = triggerValue; }
  void setReadOnly(bool readOnly) { _readonly = readOnly; }
  bool isReadonly() { return _readonly; }
  const T triggerValue() const { return _triggerValue; }
  const char *type() const { return _type; }

 protected:
  int _id;
  const char *_type;
  T _triggerValue;
  bool _triggerDisabled;
  bool _readonly;
  CompareType _compareType;
};
}  // namespace Callback

#endif