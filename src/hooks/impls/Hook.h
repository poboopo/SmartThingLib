#ifndef WATCHER_HOOK_H
#define WATCHER_HOOK_H

#define WATCHER_HOOK_TAG "watcher_hook"

#include <ArduinoJson.h>
#include <functional>

#include "hooks/comparator/Comparator.h"
#include "logs/BetterLogger.h"

#define HOOK_INFO_DOC_SIZE 512
#define MAX_HOOK_TEMPLATE_SIZE 1024
#define VALUE_DYNAMIC_PARAM "${v}"

/**
 * TODO
 * Переделать таким образом, что бы для сенсоров и состояний был отдельный шаблон
 * Таким образом разделить логику сранения значений (сделать так же проверку при установке типа сравнения)
 * Для сенсоров добавить трешхолд
 * Вынести так же получение шаблона сюда (у сенсоров и состояний будут разные шаблоны)
*/
namespace Hook {
template <typename T>
// template<ConfigurableObject<T> ????>
class Hook {
 public:
  Hook(const char *type, bool readonly)
      : _type(type),
        _readonly(readonly),
        _id(-1),
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
    if (_triggerDisabled) {
      doc["trigger"] = nullptr;
    } else {
      doc["trigger"] = _triggerValue;
    }
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
  const char *type() const { return _type; }

 protected:
  int _id;
  const char *_type;
  T _triggerValue;
  bool _triggerDisabled;
  bool _readonly;
  CompareType _compareType;
};
}  // namespace Hook

#endif