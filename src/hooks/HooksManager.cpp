#include "hooks/HooksManager.h"

#if ENABLE_HOOKS

#include <type_traits>

#include "sensors/SensorsManager.h"
#include "hooks/builders/HooksBuilder.h"
#include "hooks/watcher/Watcher.h"
#include "settings/SettingsRepository.h"

#if ENABLE_LOGGER
  const char * const _HOOKS_MANAGER_TAG = "hooks_manager";

  const char * const _errorUnkownObsType = "Unknown observable type";
  const char * const _errorObsNameMissing = "Observable name is missing";
  const char * const _errorObsTypeNotSupported = "Observable type [%s] not supported";
  const char * const _errorObsObjectMissing = "Observable object is missing";
#endif

HooksManagerClass HooksManager;

int HooksManagerClass::addHook(SensorType observableType, const char * observableName, const char * data) {
  if (observableType == UNKNOWN_OBS_TYPE) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorUnkownObsType);
    return -1;
  }
  if (observableName == nullptr || strlen(observableName) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsNameMissing);
    return -1;
  }

  #if ENABLE_STATES
  if (observableType == OBS_STATE) {
    return HooksManager.addHook<TEXT_SENSOR_TYPE>(SensorsManager.getSensor<TEXT_SENSOR_TYPE>(observableName), data);
  }
  #endif
  #if ENABLE_SENSORS 
  if (observableType == OBS_SENSOR) {
    return HooksManager.addHook<NUMBER_SENSOR_TYPE>(SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(observableName), data);
  }
  #endif
  
  st_log_error(_HOOKS_MANAGER_TAG, "This observable type is not supported");
  return -1;
}

template<typename T>
int HooksManagerClass::addHook(const Sensor<T> * observable, const char * data) {
  if (observable == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsObjectMissing);
    return -1;
  }

  if (data == nullptr || strlen(data) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook's data string can't be empty");
    return -1;
  }

  st_log_info(_HOOKS_MANAGER_TAG, "Trying to build hook for %s (observable type=%s)", observable->name(), observableTypeToStr(observable->type()));
  st_log_debug(_HOOKS_MANAGER_TAG, "Hook string: %s", data);
  
  int id = -1;
  Hook<T> * hook = HooksBuilder::build<T>(data);
  if (hook != nullptr) {
    id = addHook<T>(observable, hook);
  }
  
  if (id == -1) {
    st_log_error(_HOOKS_MANAGER_TAG, "Failed to build hook from string: %s", data);
    
    if (hook != nullptr) {
      delete hook;
    }
  }

  return id;
}

template <typename T>
int HooksManagerClass::addHook(const Sensor<T> *obj, Hook<T> *hook) {
  if (obj == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsObjectMissing);
    return -1;
  }
  if (hook == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook is missing, skipping...");
    return -1;
  }

  Watcher<T> *watcher = getWatcherOrCreate<T>(obj);
  if (watcher == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Failed to get watcher!");
    return -1;
  }

  if (!watcher->addHook(hook)) {
    st_log_error(_HOOKS_MANAGER_TAG, "Failed to add hook in watcher");
    return -1;
  }
  _hooksCount++;
  st_log_info(_HOOKS_MANAGER_TAG, "Added new hook(id=%d) for %u [%s]", hook->getId(), obj->type(), obj->name());

  return hook->getId();
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherOrCreate(const Sensor<T> *obj) {
  if (obj == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsObjectMissing);
    return nullptr;
  }

  Watcher<T> *watcher = getWatcher<T>(obj);
  if (watcher == nullptr) {
    st_log_debug(_HOOKS_MANAGER_TAG, "Creating new watcher for %u [%s]",
                 obj->type(), obj->name());
    watcher = new Watcher<T>(obj);
    if (getWatchersList<T>()->append(watcher) < 0) {
      st_log_error(_HOOKS_MANAGER_TAG,
                   "Failed to append new watcher in list for %u [%s]",
                   obj->type(), obj->name());
      delete watcher;
      return nullptr;
    }
    st_log_info(_HOOKS_MANAGER_TAG, "Added new watcher for %u [%s]",
                obj->type(), obj->name());
  } else {
    st_log_debug(_HOOKS_MANAGER_TAG, "Watcher for %u [%s] already exists!",
                 obj->type(), obj->name());
  }
  return watcher;
}

bool HooksManagerClass::deleteHook(const char *type, const char *name, int id) {
  SensorType obsType = observableTypeFromStr(type);

  if (obsType == UNKNOWN_OBS_TYPE) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorUnkownObsType);
    return -1;
  }
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsNameMissing);
    return -1;
  }

  #if ENABLE_SENSORS
  if (obsType == OBS_SENSOR) {
    return deleteHook<NUMBER_SENSOR_TYPE>(name, id);
  } 
  #endif
  #if ENABLE_STATES
  if (obsType == OBS_STATE) {
    return deleteHook<TEXT_SENSOR_TYPE>(name, id);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, _errorObsTypeNotSupported, type);
  return -1;
}

bool HooksManagerClass::updateHook(JsonDocument doc) {
  JsonObject observable = doc["observable"];
  JsonDocument hookObject = doc["hook"];

  if (observable.size() == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsObjectMissing);
    return false;
  }
  if (hookObject.size() == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook object is missing!");
    return false;
  }
  if (!hookObject[_idHookField].is<JsonVariant>()) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook id property is missing!");
    return false;
  }

  const char *name = observable["name"];
  SensorType type = observableTypeFromStr(observable["type"]);
  if (name == nullptr || type == UNKNOWN_OBS_TYPE) {
    st_log_error(_HOOKS_MANAGER_TAG, "Observable name or type is missing!");
    return false;
  }

  #if ENABLE_SENSORS
  if (type == OBS_SENSOR) {
    return updateHook<NUMBER_SENSOR_TYPE>(name, hookObject);
  }
  #endif
  #if ENABLE_STATES
  if (type == OBS_STATE) {
    return updateHook<TEXT_SENSOR_TYPE>(name, hookObject);
  }
  #endif
  st_log_error(_HOOKS_MANAGER_TAG, _errorObsTypeNotSupported, type);
  return false;
}

template <typename T>
bool HooksManagerClass::updateHook(const char *name, JsonDocument &hookObject) {
  if (!hookObject[_idHookField].is<JsonVariant>()) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "Id value in hook object is missing!");
    return false;
  }

  int id = hookObject[_idHookField];
  Hook<T> *hook = getHookFromWatcher<T>(name, id);
  if (hook == nullptr) {
    return false;
  }
  if (hook->isReadonly()) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "Hook id=%d for observable [%s] is readonly!", id, name);
    return false;
  }

  st_log_info(_HOOKS_MANAGER_TAG, "Trying to update hook id=%d for observable [%s]", id, name);

  if (hookObject[_triggerEnabledHookField].is<bool>()) {
    bool enabled = hookObject[_triggerEnabledHookField].as<bool>();
    hook->setTriggerEnabled(enabled);
    st_log_debug(_HOOKS_MANAGER_TAG, "trigger %s", enabled ? "enabled" : "disabled");
  }

  hook->setCompareType(hookObject[_compareTypeHookField].as<const char*>());
  st_log_debug(
    _HOOKS_MANAGER_TAG,
    "compareType=%s",
    compareTypeToString(hook->getCompareType()).c_str()
  );

  HooksBuilder::parseTrigger(hook, hookObject);

  hook->updateCustom(hookObject);

  st_log_info(_HOOKS_MANAGER_TAG, "Hook id=%d for observable [%s] was updated!", id, name);
  return true;
}

template <typename T>
Hook<T> *HooksManagerClass::getHookFromWatcher(const char *name, int id) {
  Watcher<T> *watcher = getWatcherByObservableName<T>(name);
  if (watcher == nullptr) {
    return nullptr;
  }
  Hook<T> *hook = watcher->getHookById(id);
  if (hook == nullptr) {
    st_log_warning(_HOOKS_MANAGER_TAG, "Can't find hook id=%d for observable [%s]", id, name);
    return nullptr;
  }
  return hook;
}

template <typename T>
bool HooksManagerClass::deleteHook(const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of observable is missing!");
    return false;
  }
  st_log_warning(_HOOKS_MANAGER_TAG,
               "Trying to delete observable [%s]'s hook id=%d", name, id);
  Watcher<T> *watcher = getWatcherByObservableName<T>(name);
  if (watcher == nullptr || !watcher->removeHook(id)) {
    return false;
  }
  _hooksCount--;
  st_log_warning(_HOOKS_MANAGER_TAG,
                 "Hook № %d of observable [%s] was deleted", id, name);
  if (watcher->haveHooks()) {
    return true;
  }
  st_log_debug(_HOOKS_MANAGER_TAG,
               "No hooks left for observable [%s], removing watcher!",
               name);
  if (!getWatchersList<T>()->remove(watcher)) {
    return false;
  }
  delete watcher;
  st_log_warning(_HOOKS_MANAGER_TAG, "Watcher for observable [%s] removed!",
                 name);
  return true;
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcher(const Sensor<T> *observable) {
  return getWatchersList<T>()->findValue([observable](Watcher<T> *current) {
    return current->getObservable() == observable;
  });
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherByObservableName(const char *name) {
  return getWatchersList<T>()->findValue([name](Watcher<T> *current) {
    return strcmp(current->getObservable()->name(), name) == 0;
  });
}

void HooksManagerClass::check() {
  #if ENABLE_SENSORS 
  if (_sensorsWatchers.size() > 0) {
    checkWatchers<NUMBER_SENSOR_TYPE>();
  }
  #endif
  #if ENABLE_STATES
  if (_statesWatchers.size() > 0) {
    checkWatchers<TEXT_SENSOR_TYPE>();
  }
  #endif
}

template <typename T>
void HooksManagerClass::checkWatchers() {
  getWatchersList<T>()->forEach([](Watcher<T> *current) { current->check(); });
}

boolean HooksManagerClass::callHook(const char * type, const char * name, int id, String value) {
  SensorType observableType = observableTypeFromStr(type);

  if (observableType == UNKNOWN_OBS_TYPE) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorUnkownObsType);
    return false;
  }
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsNameMissing);
    return false;
  }

  boolean emptyValue = value.isEmpty();
  st_log_info(
    _HOOKS_MANAGER_TAG,
    "Trying to call hook type=%s, name=%s, id=%d, value=%s",
    type,
    name,
    id,
    emptyValue ? "" : value.c_str()
  );

  #if ENABLE_SENSORS
  if (observableType == OBS_SENSOR) {
    return callWatcherHook<NUMBER_SENSOR_TYPE>(name, id, emptyValue ? 0 : value.toInt(), emptyValue);
  }
  #endif
  #if ENABLE_STATES
  if (observableType == OBS_STATE) {
    return callWatcherHook<TEXT_SENSOR_TYPE>(name, id, value, emptyValue);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "Type %s is not supported!", type);
  return false;
}

template <typename T>
boolean HooksManagerClass::callWatcherHook(const char * name, int id, T value, boolean emptyValue) {
  Watcher<T> * watcher = getWatcherByObservableName<T>(name);
  if (watcher == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Can't find watcher for observable with name=%s", name);
    return false;
  }
  Hook<T> * hook = watcher->getHookById(id);
  if (hook == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Can't find hook for observable %s by id=%d", name, id);
    return false;
  }
  const Sensor<T> * obs = watcher->getObservable();
  if (obs == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "OBSERVABLE NULLPTR! HOW???");
    return false;
  }
  if (emptyValue) {
    st_log_info(_HOOKS_MANAGER_TAG, "Extracting value and calling hook");
    T v = obs->provideValue();
    hook->call(v);
  } else {
    st_log_info(_HOOKS_MANAGER_TAG, "Calling hook with provided value");
    hook->call(value);
  }
  return true;
}

void HooksManagerClass::loadFromSettings() {
  String fromSettings = SettingsRepository.getHooks();

  if (fromSettings.isEmpty()) {
    st_log_info(_HOOKS_MANAGER_TAG, "No hooks in settings");
    return;
  }

  bool failedBuild = false;
  int address = 0;
  int dataLength = fromSettings.length();
  const char * data = fromSettings.c_str();

  st_log_debug(_HOOKS_MANAGER_TAG, "Building hooks from settings");
  while(address < dataLength) {
    SensorType type = static_cast<SensorType>(data[address] - '0');
    String name;

    for (address = address + 1; address < dataLength; address++) {
      if (data[address] == '\t') {
        break;
      }
      name += data[address];
    }

    address++;
    if (type == OBS_SENSOR) {
      failedBuild = loadHooks<NUMBER_SENSOR_TYPE>(SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(name.c_str()), data, &address, dataLength) || failedBuild;
    } else if (type == OBS_STATE) {
      failedBuild = loadHooks<TEXT_SENSOR_TYPE>(SensorsManager.getSensor<TEXT_SENSOR_TYPE>(name.c_str()), data, &address, dataLength) || failedBuild;
    } else {
      st_log_error(_HOOKS_MANAGER_TAG, _errorUnkownObsType);
    }

    address++;
  }

  st_log_debug(_HOOKS_MANAGER_TAG, "Hooks loaded! Total count: %d", _hooksCount);

  if (failedBuild) {
    st_log_warning(_HOOKS_MANAGER_TAG, "Have some ghost hooks to delete (Failed build). Trying to save correct hooks list");
    if (saveInSettings()) {
      st_log_warning(_HOOKS_MANAGER_TAG, "Ghost hooks removed");
    }
  }
}

template<typename T>
bool HooksManagerClass::loadHooks(const Sensor<T> * observable, const char * data, int * address, int length) {
  bool res = false;
  String buff; // todo remove, pass hook length

  for (; (*address) < length; (*address)++) {
    if (data[(*address)] == '\t' || data[(*address)] == '\n') {
      res = addHook(observable, buff.c_str()) == -1 || res;
      buff.clear();
    } else {
      buff += data[(*address)];
    }

    if (data[(*address)] == '\n') {
      break;
    }
  }

  return res;
}

bool HooksManagerClass::saveInSettings() {
  st_log_debug(_HOOKS_MANAGER_TAG, "Saving hooks");
  String data = "";

  #if ENABLE_STATES
  _statesWatchers.forEach([&](Watcher<TEXT_SENSOR_TYPE> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    data += watcher->toString();
    data += "\n";
  });
  #endif
  #if ENABLE_SENSORS 
  _sensorsWatchers.forEach([&](Watcher<NUMBER_SENSOR_TYPE> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    data += watcher->toString();
    data += "\n";
  });
  #endif

  return SettingsRepository.setHooks(data);
}

JsonDocument HooksManagerClass::getObservableHooksJson(const char *type, const char *name) {
  SensorType observableType = observableTypeFromStr(type);
  JsonDocument doc;

  if (observableType == UNKNOWN_OBS_TYPE) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorUnkownObsType);
    return doc;
  }
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorObsNameMissing);
    return doc;
  }

  #if ENABLE_SENSORS 
  if (observableType == OBS_SENSOR) {
    return getObservableHooksJson<NUMBER_SENSOR_TYPE>(name);
  } 
  #endif
  #if ENABLE_STATES
  if (observableType == OBS_STATE) {
    return getObservableHooksJson<TEXT_SENSOR_TYPE>(name);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, _errorObsTypeNotSupported, type);
  return doc;
}

template <typename T>
JsonDocument HooksManagerClass::getObservableHooksJson(const char *name) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of observable is missing!");
  } else {
    Watcher<T> *watcher = getWatcherByObservableName<T>(name);
    if (watcher != nullptr) {
      return watcher->getObservableHooksJson();
    }
  }
  JsonDocument doc;
  return doc;
}

#if ENABLE_STATES
  template <>
  List<Watcher<TEXT_SENSOR_TYPE>> *HooksManagerClass::getWatchersList() {
    return &_statesWatchers;
  }
#endif

#if ENABLE_SENSORS
  template <>
  List<Watcher<NUMBER_SENSOR_TYPE>> *HooksManagerClass::getWatchersList() {
    return &_sensorsWatchers;
  }
#endif

#endif