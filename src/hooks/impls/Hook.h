#ifndef WATCHER_HOOK_H
#define WATCHER_HOOK_H

#include <ArduinoJson.h>
#include <functional>

#include "Features.h"
#include "logs/BetterLogger.h"
#include "hooks/impls/HookConstans.h"
#include "sensors/Sensor.h"

#define CHECK_HOOK_DATA_TYPE typename std::enable_if<std::is_same<T, NUMBER_SENSOR_DATA_TYPE>::value || std::is_same<T, TEXT_SENSOR_DATA_TYPE>::value>::type* = nullptr

#if ENABLE_NUMBER_SENSORS && ENABLE_TEXT_SENSORS
#define SELECT_HOOK_BASE_CLASS std::conditional<std::is_same<T, NUMBER_SENSOR_DATA_TYPE>::value, NumberSensorHook, TextSensorHook>::type
  #elif ENABLE_NUMBER_SENSORS
#define SELECT_HOOK_BASE_CLASS NumberSensorHook
#elif ENABLE_TEXT_SENSORS
  #define SELECT_HOOK_BASE_CLASS TextSensorHook
#endif

template <typename T>
class Hook {
  static_assert(std::is_same<T, NUMBER_SENSOR_DATA_TYPE>::value || std::is_same<T, TEXT_SENSOR_DATA_TYPE>::value);

  public:
    Hook(HookType type, int id = -1, bool triggerEnabled = false, CompareType compare = EQ, bool readOnly = true)
        : _type(type),
          _id(id),
          _triggerEnabled(triggerEnabled),
          _compareType(compare),
          _readonly(readOnly) {};

    virtual ~Hook() {};

    // todo make value const
    virtual bool accept(T &value) = 0;
    virtual void call(T &value) = 0;
    virtual void updateCustom(JsonDocument &doc) {};

    void setId(int id) { _id = id; }
    const int getId() const { return _id; }
    void setCompareType(const char * type) {
      _compareType = compareTypeFromString(type, CompareType::EQ);
    }
    void setCompareType(CompareType type) { _compareType = type; }
    CompareType getCompareType() const { return _compareType; }
    void setTriggerValue(T triggerValue) { _triggerValue = triggerValue; }
    void setTriggerEnabled(bool enabled) { _triggerEnabled = enabled; }
    void setReadOnly(bool readOnly) { _readonly = readOnly; }
    bool isReadonly() const { return _readonly; }

    String toString() {
      String trigger = triggerString();
      String customValues = customValuesString();

      char * result = (char *) malloc(5 + trigger.length() + 1 + customValues.length() + 1);
      sprintf(
        result,
        "%d%02d%d%d%s;%s",
        _type,
        _id,
        _compareType,
        _triggerEnabled,
        trigger.c_str(),
        customValues.c_str()
      );

      String tmp = result;
      free(result);
      return tmp;
    }

    JsonDocument toJson() const {
      JsonDocument doc;
      doc[_idHookField] = _id;
      doc[_readonlyHookField] = _readonly;
      doc[_typeHookField] = hookTypeToStr(_type);
      doc[_triggerEnabledHookField] = _triggerEnabled;
      doc[_triggerHookField] = _triggerValue;
      doc[_compareTypeHookField] = compareTypeToString(_compareType);

      addTypeSpecificValues(doc);
      populateJsonWithCustomValues(doc);
      return doc;
    }

  protected:
    HookType _type;
    int _id;
    bool _triggerEnabled;
    CompareType _compareType;
    T _triggerValue;
    bool _readonly;

    virtual String triggerString() = 0;
    virtual String customValuesString() = 0;
    virtual void addTypeSpecificValues(JsonDocument &doc) const {};
    virtual void populateJsonWithCustomValues(JsonDocument &doc) const {};
};

#if ENABLE_NUMBER_SENSORS
class NumberSensorHook: public Hook<NUMBER_SENSOR_DATA_TYPE> {
  public:
    NumberSensorHook(HookType type): Hook<NUMBER_SENSOR_DATA_TYPE>(type), _threshold(0), _previousValue(0) {};
    bool accept(NUMBER_SENSOR_DATA_TYPE &value) {
      if (_threshold != 0 && abs(value - _previousValue) < _threshold) {
        return false;
      }
      _previousValue = value;

      if (!_triggerEnabled) {
        return true;
      }

      switch (_compareType) {
        case CompareType::EQ:
          return value == _triggerValue;
        case CompareType::NEQ:
          return value != _triggerValue;
        case CompareType::GTE:
          return value >= _triggerValue;
        case CompareType::LTE:
          return value <= _triggerValue;
        default:
          return false;
      }
    }

    void setThreshold(NUMBER_SENSOR_DATA_TYPE threshold) {
      _threshold = threshold;
    }
  private:
    NUMBER_SENSOR_DATA_TYPE _threshold;
    NUMBER_SENSOR_DATA_TYPE _previousValue;

  protected:
    void addTypeSpecificValues(JsonDocument &doc) const {
      doc[_thresholdHookField] = _threshold;
    }

    String triggerString() {
      int size = snprintf(NULL, 0, "%d:%d", _triggerValue, _threshold);
      char buff[size + 1];
      sprintf(buff, "%d_%d", _triggerValue, _threshold);
      String tmp = buff;
      return tmp;
    }
};
#endif

#if ENABLE_TEXT_SENSORS
class TextSensorHook: public Hook<TEXT_SENSOR_DATA_TYPE> {
  public:
    TextSensorHook(HookType type): Hook<TEXT_SENSOR_DATA_TYPE>(type) {};
    bool accept(TEXT_SENSOR_DATA_TYPE &value) {
      if (!_triggerEnabled) {
        return true;
      }
      switch (_compareType) {
        case CompareType::EQ:
          return _triggerValue.equals(value);
        case CompareType::NEQ:
          return _triggerValue.equals(value);
        default:
          return false;
      }
    }
  protected:
    TEXT_SENSOR_DATA_TYPE triggerString() {
      String copy = _triggerValue;
      copy.replace(";", "|;");
      return _triggerValue;
    }
};
#endif

#endif