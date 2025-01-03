#ifndef WATCHER_H
#define WATCHER_H

#include "Features.h"

#if ENABLE_HOOKS

#include <ArduinoJson.h>
#include <functional>
#include <list>

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
    typename std::list<Hook<T>*>::iterator it;
    for (it = _hooks.begin(); it != _hooks.end(); ++it) {
      Hook<T> *current = *it;
      if (current != nullptr && current->accept(value)) {
        st_log_debug(
          _WATCHER_TAG,
          "Calling hook [id=%d] for sensor %s",
          current->getId(),
          _sensor->name()
        );
        current->call(value);
      }
    }
  }
 
  Hook<T> *getHookById(int id) {
    if (id < 0) {
      return nullptr;
    }

    auto it = std::find_if(_hooks.begin(), _hooks.end(), [id](const Hook<T> * hook) {
      return hook->getId() == id;
    });

    if (it == _hooks.end()) {
      return nullptr;
    }
    return *it;
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

    _hooks.push_back(hook);
    return true;
  }
   
  bool removeHook(int id) {
    if (id < 0) {
      st_log_error(_WATCHER_TAG, "Failed to remove hook - id negative!");
      return false;
    }

    auto it = std::find_if(_hooks.begin(), _hooks.end(), [id](const Hook<T> * hook) {
      return hook->getId() == id;
    });
    if (it == _hooks.end()) {
      st_log_error(_WATCHER_TAG,
                  "Failed to remove hook - can't find hook with id %d",
                  id);
      return false;
    }

    Hook<T> *hook = *it;
    if (hook->isReadonly()) {
      st_log_error(_WATCHER_TAG, "This hook is readonly!");
      return false;
    }

    delete hook;
    _hooks.erase(it);
    st_log_warning(_WATCHER_TAG, "Hook %d removed", id);
    return true;
  }
 
  String toString() {
    if (_hooks.size() == 0) {
      return "";
    }

    String result = _sensor->name();

    typename std::list<Hook<T>*>::iterator it;
    for (it = _hooks.begin(); it != _hooks.end(); ++it) {
      if ((*it)->isReadonly()) {
        continue;
      }

      result += "\t";
      result += (*it)->toString();
    }

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

  // todo remove aduino json usage
  JsonDocument getSensorHooksJson() {
    JsonDocument doc;
    doc.to<JsonArray>();
    if (_hooks.size() == 0) {
      return doc;
    }

    typename std::list<Hook<T>*>::iterator it;
    for (it = _hooks.begin(); it != _hooks.end(); ++it) {
      if ((*it)->isReadonly()) {
        continue;
      }

      doc.add((*it)->toJson());
    }

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
  std::list<Hook<T>*> _hooks;

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