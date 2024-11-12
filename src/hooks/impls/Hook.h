#ifndef WATCHER_HOOK_H
#define WATCHER_HOOK_H

#include <ArduinoJson.h>
#include <functional>

#include "Features.h"
#include "hooks/comparator/Comparator.h"
#include "logs/BetterLogger.h"

static const char * _lambdaHookStr = "lambda";
static const char * _actionHookStr = "action";
static const char * _httpHookStr = "http";
static const char * _notificationHookStr = "notification";

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
      return _lambdaHookStr;
    case ACTION_HOOK:
      return _actionHookStr;
    case HTTP_HOOK:
      return _httpHookStr;
    case NOTIFICATION_HOOK:
      return _notificationHookStr;
    default:
      return "unknown";
  }
}

inline HookType hookTypeFromStr(const char * type) {
  if (type == nullptr) {
    return UNKNOWN_HOOK;
  }
  if (strcmp(type, _actionHookStr) == 0) {
    return ACTION_HOOK;
  }
  if (strcmp(type, _httpHookStr) == 0) {
    return HTTP_HOOK;
  }
  if (strcmp(type, _notificationHookStr) == 0) {
    return NOTIFICATION_HOOK;
  }
  if (strcmp(type, _lambdaHookStr) == 0) {
    return LAMBDA_HOOK;
  }
  return UNKNOWN_HOOK;
}

template <typename T>
class Hook {
  public:
    Hook(HookType type, bool readonly, int id = -1, bool triggerDisabled = true, CompareType compare = EQ)
        : _type(type),
          _readonly(readonly),
          _id(id),
          _compareType(compare),
          _triggerDisabled(triggerDisabled){};

    virtual ~Hook() {};

    // todo make value const
    virtual bool accept(T &value) = 0;
    virtual void call(T &value) = 0;
    virtual void updateCustom(JsonObject doc) {};
    virtual void populateJsonWithCustomValues(JsonDocument &doc, boolean shortJson) const {};
    virtual void disableTrigger() { _triggerDisabled = true; };
    virtual void enableTrigger() { _triggerDisabled = false; };
    virtual bool triggerDisabled() const { return _triggerDisabled; };

    virtual JsonDocument toJson(bool shortJson) const {
      JsonDocument doc;
      doc["id"] = _id;
      doc["readonly"] = _readonly;
      doc["type"] = hookTypeToStr(_type);
      doc["triggerEnabled"] = !_triggerDisabled;
      doc["trigger"] = _triggerValue;
      doc["compareType"] = compareTypeToString(_compareType);

      populateJsonWithCustomValues(doc, shortJson);
      return doc;
    }

    void setId(int id) { _id = id; }
    const int getId() const { return _id; }
    void setCompareType(const char * type) {
      _compareType = compareTypeFromString(type, CompareType::EQ);
    }
    void setCompareType(CompareType type) { _compareType = type; }
    CompareType getCompareType() const { return _compareType; }
    void setTriggerValue(T triggerValue) { _triggerValue = triggerValue; }
    void setReadOnly(bool readOnly) { _readonly = readOnly; }
    bool isReadonly() const { return _readonly; }
    HookType type() const { return _type; }

  protected:
    HookType _type;
    bool _readonly;
    int _id;
    CompareType _compareType;
    bool _triggerDisabled;
    T _triggerValue;
};

#if ENABLE_SENSORS
class SensorHook: public Hook<int16_t> {
  public:
    SensorHook(HookType type, bool readonly): Hook<int16_t>(type, readonly) {};
    bool accept(int16_t &value) {
      if (abs(value - _previousValue) < _threshold) {
        return false;
      }
      _previousValue = value;

      if (_triggerDisabled) {
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
    
    JsonDocument toJson(bool shortJson) {
      JsonDocument doc;
      doc["id"] = _id;
      doc["readonly"] = _readonly;
      doc["type"] = _type;
      if (_triggerDisabled) {
        doc["trigger"] = nullptr;
      } else {
        doc["trigger"] = _triggerValue;
      }
      doc["threshold"] = _threshold;
      doc["compareType"] = compareTypeToString(_compareType);

      populateJsonWithCustomValues(doc, shortJson);
      return doc;
    }

    void disableTrigger() {
      _triggerDisabled = true;
      _triggerValue = 0;
    }
    void enableTrigger() {
      _triggerDisabled = false;
    }

    void setThreshold(int16_t threshold) {
      _threshold = threshold;
    }
  private:
    int16_t _threshold;
    int16_t _previousValue;
};
#endif
#if ENABLE_STATES
class StateHook: public Hook<String> {
  public:
    StateHook(HookType type, bool readonly): Hook<String>(type, readonly) {};
    bool accept(String &value) {
      if (_triggerDisabled) {
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

    void disableTrigger() {
      _triggerDisabled = true;
      _triggerValue = "";
    }
    void enableTrigger() {
      _triggerDisabled = false;
    }

};
#endif

#endif