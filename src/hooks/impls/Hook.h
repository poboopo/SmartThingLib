#ifndef WATCHER_HOOK_H
#define WATCHER_HOOK_H

#include <ArduinoJson.h>
#include <functional>

#include "Features.h"
#include "hooks/comparator/Comparator.h"
#include "logs/BetterLogger.h"
#include "observable/ObservableObjects.h"

#define CHECK_HOOK_DATA_TYPE typename std::enable_if<std::is_same<T, SENSOR_DATA_TYPE>::value || std::is_same<T, STATE_DATA_TYPE>::value>::type* = nullptr
#define SELECT_HOOK_BASE_CLASS std::conditional<std::is_same<T, SENSOR_DATA_TYPE>::value, SensorHook, StateHook>::type

static const char * _lambdaHookType = "lambda";
static const char * _actionHookType = "action";
static const char * _httpHookType = "http";
static const char * _notificationHookType = "notification";

static const char * _idHookField = "id";
static const char * _readonlyHookField = "readonly";
static const char * _typeHookField = "type";
static const char * _triggerEnabledHookField = "triggerEnabled";
static const char * _triggerHookField = "trigger";
static const char * _compareTypeHookField = "compareType";
static const char * _thresholdHookField = "threshold";

enum HookType {
  UNKNOWN_HOOK,
  LAMBDA_HOOK,
  ACTION_HOOK,
  HTTP_HOOK,
  NOTIFICATION_HOOK
};

inline const char * hookTypeToStr(HookType type) {
  switch (type) {
    case LAMBDA_HOOK:
      return _lambdaHookType;
    case ACTION_HOOK:
      return _actionHookType;
    case HTTP_HOOK:
      return _httpHookType;
    case NOTIFICATION_HOOK:
      return _notificationHookType;
    default:
      return "unknown";
  }
}

inline HookType hookTypeFromStr(const char * type) {
  if (type == nullptr) {
    return UNKNOWN_HOOK;
  }
  if (strcmp(type, _actionHookType) == 0) {
    return ACTION_HOOK;
  }
  if (strcmp(type, _httpHookType) == 0) {
    return HTTP_HOOK;
  }
  if (strcmp(type, _notificationHookType) == 0) {
    return NOTIFICATION_HOOK;
  }
  if (strcmp(type, _lambdaHookType) == 0) {
    return LAMBDA_HOOK;
  }
  return UNKNOWN_HOOK;
}

template <typename T>
class Hook {
  static_assert(std::is_same<T, SENSOR_DATA_TYPE>::value || std::is_same<T, STATE_DATA_TYPE>::value);

  public:
    Hook(HookType type, int id = -1, bool triggerEnabled = false, CompareType compare = EQ)
        : _type(type),
          _id(id),
          _compareType(compare),
          _triggerEnabled(triggerEnabled),
          _readonly(true) {};

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

    virtual JsonDocument toJson() const {
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
    bool _readonly;
    int _id;
    CompareType _compareType;
    bool _triggerEnabled;
    T _triggerValue;

    virtual String triggerString() = 0;
    virtual String customValuesString() = 0;
    virtual void addTypeSpecificValues(JsonDocument &doc) const {};
    virtual void populateJsonWithCustomValues(JsonDocument &doc) const {};
};

#if ENABLE_SENSORS
class SensorHook: public Hook<SENSOR_DATA_TYPE> {
  public:
    SensorHook(HookType type): Hook<SENSOR_DATA_TYPE>(type), _threshold(0), _previousValue(0) {};
    bool accept(SENSOR_DATA_TYPE &value) {
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

    void setThreshold(SENSOR_DATA_TYPE threshold) {
      _threshold = threshold;
    }
  private:
    SENSOR_DATA_TYPE _threshold;
    SENSOR_DATA_TYPE _previousValue;

  protected:
    void addTypeSpecificValues(JsonDocument &doc) const {
      doc[_thresholdHookField] = _threshold;
    }

    String triggerString() {
      int size = snprintf(NULL, 0, "%d:%d", _triggerValue, _threshold);
      char * buff = (char *) malloc(size + 1);
      sprintf(buff, "%d_%d", _triggerValue, _threshold);
      String tmp = buff;
      free(buff);
      return tmp;
    }
};
#endif

#if ENABLE_STATES
class StateHook: public Hook<STATE_DATA_TYPE> {
  public:
    StateHook(HookType type): Hook<STATE_DATA_TYPE>(type) {};
    bool accept(STATE_DATA_TYPE &value) {
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
    STATE_DATA_TYPE triggerString() {
      String copy = _triggerValue;
      copy.replace(";", "|;");
      return _triggerValue;
    }
};
#endif

#endif