#ifndef WATCHER_H
#define WATCHER_H

#include "Features.h"

#if ENABLE_HOOKS

#include <ArduinoJson.h>
#include <functional>

#include "hooks/impls/Hook.h"
#include "sensors/Sensor.h"
#include "logs/BetterLogger.h"
#include "utils/List.h"

const char * const _WATCHER_TAG = "watcher";

/*
    Класс наблюдатель за сенсорами
    T - тип данных, которые хранит в себе сенсор
*/

template <typename T>
class Watcher {
 public:
  Watcher(const Sensor<T> *sensor)
      : _sensor(sensor),
        _hookIdSequence(0) {
    setInitialValue();
  };
  ~Watcher() {};

  bool check();

  void callHooks(T &value) {
    if (_hooks.size() == 0) {
      return;
    }
    _hooks.forEach([&, this](Hook<T> *current) {
      if (current != nullptr && current->accept(value)) {
        st_log_debug(
          _WATCHER_TAG,
          "Calling hook [id=%d] for sensor %s",
          current->getId(),
          _sensor->name()
        );
        current->call(value);
      }
    });
  }
 
  Hook<T> *getHookById(int id) {
    if (id < 0) {
      return nullptr;
    }
    return _hooks.findValue([&](Hook<T> *hook) {
      return hook->getId() == id;
    });
  }
   
  bool addHook(Hook<T> *hook) {
    if (hook == nullptr) {
      st_log_error(_WATCHER_TAG, "Hook is missing!");
      return false;
    }

    if (hook->isReadonly()) {
      hook->setId(-1);
    } else if (hook->getId() < 0) {
      int id = getNextHookId();
      if (id < 0) {
        st_log_error(_WATCHER_TAG, "Failed to generate new id for hook");
        return false;
      }
      hook->setId(id);
    } else if (getHookById(hook->getId()) != nullptr) {
      st_log_error(_WATCHER_TAG, "Hook with id=%d already exists!", hook->getId());
      return false;
    }

    _hooks.append(hook);
    return true;
  }
   
  bool removeHook(int id) {
    if (id < 0) {
      st_log_error(_WATCHER_TAG, "Failed to remove hook - id negative!");
      return false;
    }
    Hook<T> *hook = getHookById(id);
    if (hook == nullptr) {
      st_log_error(_WATCHER_TAG,
                  "Failed to remove hook - can't find hook with id %d",
                  id);
      return false;
    }
    if (hook->isReadonly()) {
      st_log_error(_WATCHER_TAG, "This hook is readonly!");
      return false;
    }
    if (_hooks.remove(hook)) {
      delete hook;
      return true;
    }
    st_log_error(_WATCHER_TAG, "Failed to remove hook from list");
    return false;
  }
 
  String toString() {
    if (_hooks.size() == 0) {
      return "";
    }

    String result = _sensor->name();

    _hooks.forEach([&](Hook<T> * hook) {
      if (hook == nullptr || hook->isReadonly()) {
        return;
      }

      result += "\t";
      result += hook->toString();
    });

    return result;
  }

  JsonDocument toJson() {
    JsonDocument doc;
    if (_hooks.size() == 0) {
      return doc;
    }
    JsonDocument hooks = getSensorHooksJson();
    doc["sensor"] = _sensor->name();
    doc["hooks"] = hooks;
    return doc;
  }
  JsonDocument getSensorHooksJson() {
    JsonDocument doc;
    doc.to<JsonArray>();
    if (_hooks.size() == 0) {
      st_log_debug(_WATCHER_TAG, "No hook's, creating empty array");
      return doc;
    }
    _hooks.forEach([&](Hook<T> *current) {
      if (current == nullptr && current->isReadonly()) {
        return;
      }
      doc.add(current->toJson());
    });
    return doc;
  }

  const Sensor<T> *getSensor() const {
    return _sensor;
  };

  bool haveHooks() { return _hooks.size() != 0; }

  uint8_t hooksCount() { return _hooks.size(); }

 protected:
  const Sensor<T> *_sensor;
  T _oldValue;
  List<Hook<T>> _hooks;

 private:
  int _hookIdSequence;

  int getNextHookId() {
    bool res = false;
    uint8_t attempts = 20;
    while (!res && attempts != 0) {
      _hookIdSequence++;
      res = getHookById(_hookIdSequence) == nullptr;
      attempts--;
    }
    if (attempts == 0) {
      return -1;
    }
    return _hookIdSequence;
  }

  void setInitialValue();
};

#endif
#endif