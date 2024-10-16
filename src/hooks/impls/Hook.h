#ifndef WATCHER_HOOK_H
#define WATCHER_HOOK_H

#include <ArduinoJson.h>
#include <functional>

#include "Features.h"
#include "hooks/comparator/Comparator.h"
#include "logs/BetterLogger.h"

namespace Hook {
  template <typename T>
  class Hook {
    public:
      Hook(const char *type, bool readonly, int id = -1, bool triggerDisabled = true, CompareType compare = EQ)
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
      virtual void addCustomJsonValues(JsonDocument &doc, boolean shortJson) {};
      virtual void disableTrigger() { _triggerDisabled = true; };
      virtual void enableTrigger() { _triggerDisabled = false; };

      virtual JsonDocument toJson(bool shortJson) {
        JsonDocument doc;
        doc["id"] = _id;
        doc["readonly"] = _readonly;
        doc["type"] = _type;
        if (_triggerDisabled) {
          doc["trigger"] = nullptr;
        } else {
          doc["trigger"] = _triggerValue;
        }
        doc["compareType"] = compareTypeToString(_compareType);

        addCustomJsonValues(doc, shortJson);
        return doc;
      }

      void setId(int id) { _id = id; }
      const int getId() const { return _id; }
      void setCompareType(String type) {
        _compareType = compareTypeFromString(type, CompareType::EQ);
      }
      void setCompareType(CompareType type) { _compareType = type; }
      void setTriggerValue(T triggerValue) { _triggerValue = triggerValue; }
      void setReadOnly(bool readOnly) { _readonly = readOnly; }
      bool isReadonly() { return _readonly; }
      const char *type() const { return _type; }

    protected:
      const char *_type;
      bool _readonly;
      int _id;
      CompareType _compareType;
      bool _triggerDisabled;
      T _triggerValue;
  };

  #if ENABLE_SENSORS
  class SensorHook: public Hook<int16_t> {
    public:
      SensorHook(const char *type, bool readonly): Hook<int16_t>(type, readonly) {};
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

        addCustomJsonValues(doc, shortJson);
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
      StateHook(const char *type, bool readonly): Hook<String>(type, readonly) {};
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
}  // namespace Hook

#endif