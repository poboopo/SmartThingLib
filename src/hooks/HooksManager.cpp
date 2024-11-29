#include "hooks/HooksManager.h"

#if ENABLE_HOOKS 

#include <type_traits>

#include "observable/ObservablesManager.h"
#include "hooks/builders/HooksBuilder.h"
#include "hooks/watchers/DeviceStateWatcher.h"
#include "hooks/watchers/SensorWatcher.h"
#include "settings/SettingsRepository.h"

const char * const _HOOKS_MANAGER_TAG = "hooks_manager";

HooksManagerClass HooksManager;

int HooksManagerClass::addHook(ObservableType observableType, const char * observableName, const char * data) {
  if (observableType == UNKNOWN_OBS_TYPE) {
    st_log_error(_HOOKS_MANAGER_TAG, "Unkown observable type");
    return -1;
  }

  #if ENABLE_STATES
  if (observableType == OBS_STATE) {
    return HooksManager.addHook<STATE_DATA_TYPE>(ObservablesManager.getObservableObject<STATE_DATA_TYPE>(observableName), data);
  }
  #endif
  #if ENABLE_SENSORS 
  if (observableType == OBS_SENSOR) {
    return HooksManager.addHook<SENSOR_DATA_TYPE>(ObservablesManager.getObservableObject<SENSOR_DATA_TYPE>(observableName), data);
  }
  #endif
  
  st_log_error(_HOOKS_MANAGER_TAG, "This observable type is not supported");
  return -1;
}

template<typename T>
int HooksManagerClass::addHook(const ObservableObject<T> * observable, const char * data) {
  if (observable == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Observable obejct is missing");
    return -1;
  }

  if (data == nullptr || strlen(data) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook's data string can't be empty");
    return -1;
  }

  st_log_info(_HOOKS_MANAGER_TAG, "Trying to build hook for %s (observable type=%s)", observable->name, observableTypeToStr(observable->type));
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

template <class T>
int HooksManagerClass::addHook(const ObservableObject<T> *obj, Hook<T> *hook) {
  if (obj == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "Observable object is missing, cancelling hook creation");
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
  st_log_info(_HOOKS_MANAGER_TAG, "Added new hook(id=%d) for %u [%s]", hook->getId(), obj->type, obj->name);

  return hook->getId();
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherOrCreate(
    const ObservableObject<T> *obj) {
  if (obj == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Observable object is missing");
    return nullptr;
  }

  List<Watcher<T>> *watchersList = getWatchersList<T>();
  if (watchersList == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Failed to define watchers list");
    return nullptr;
  }

  Watcher<T> *watcher = getWatcher<T>(watchersList, obj);
  if (watcher == nullptr) {
    st_log_debug(_HOOKS_MANAGER_TAG, "Creating new watcher for %u [%s]",
                 obj->type, obj->name);
    watcher = createWatcher<T>(obj);
    if (watchersList->append(watcher) < 0) {
      st_log_error(_HOOKS_MANAGER_TAG,
                   "Failed to append new watcher in list for %u [%s]",
                   obj->type, obj->name);
      delete watcher;
      return nullptr;
    }
    st_log_info(_HOOKS_MANAGER_TAG, "Added new watcher for %u [%s]",
                obj->type, obj->name);
  } else {
    st_log_debug(_HOOKS_MANAGER_TAG, "Watcher for %u [%s] already exists!",
                 obj->type, obj->name);
  }
  return watcher;
}

#if ENABLE_STATES
template <>
Watcher<STATE_DATA_TYPE> *HooksManagerClass::createWatcher(
    const ObservableObject<STATE_DATA_TYPE> *obj) {
  return new DeviceStateWatcher((DeviceState *)obj);
}

template <>
List<Watcher<STATE_DATA_TYPE>> *HooksManagerClass::getWatchersList() {
  return &_statesWatchers;
}

#endif

#if ENABLE_SENSORS 
template <>
Watcher<SENSOR_DATA_TYPE> *HooksManagerClass::createWatcher(const ObservableObject<SENSOR_DATA_TYPE> *obj) {
  return new SensorWatcher((Sensor *)obj);
}

template <>
List<Watcher<SENSOR_DATA_TYPE>> *HooksManagerClass::getWatchersList() {
  return &_sensorsWatchers;
}

#endif

bool HooksManagerClass::deleteHook(const char *type, const char *name,
                                           int id) {
  if (type == nullptr || strlen(type) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Type of observable is missing!");
    return -1;
  }

  #if ENABLE_SENSORS
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return deleteHookFromList<SENSOR_DATA_TYPE>(&_sensorsWatchers, name, id);
  } 
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return deleteHookFromList<STATE_DATA_TYPE>(&_statesWatchers, name, id);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  return -1;
}

bool HooksManagerClass::updateHook(JsonDocument doc) {
  JsonObject observable = doc["observable"];
  JsonDocument hookObject = doc["hook"];

  if (observable.size() == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Observable object is missing!");
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
  const char *type = observable["type"];
  if (name == nullptr || type == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Observable name or type is missing!");
    return false;
  }

  #if ENABLE_SENSORS
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return updateHook<SENSOR_DATA_TYPE>(&_sensorsWatchers, name, hookObject);
  }
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return updateHook<STATE_DATA_TYPE>(&_statesWatchers, name, hookObject);
  }
  #endif
  st_log_error(_HOOKS_MANAGER_TAG, "Observable type [%s] not supported!",
               type);
  return false;
}

template <typename T>
bool HooksManagerClass::updateHook(List<Watcher<T>> *list,
                                           const char *name,
                                           JsonDocument &hookObject) {
  if (!hookObject[_idHookField].is<JsonVariant>()) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "Id value in hook object is missing!");
    return false;
  }

  int id = hookObject[_idHookField];
  Hook<T> *hook = getHookFromWatcherList(list, name, id);
  if (hook == nullptr) {
    return false;
  }
  if (hook->isReadonly()) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "Hook %d for observable [%s] is readonly!", id, name);
    return false;
  }

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
Hook<T> *HooksManagerClass::getHookFromWatcherList(
    List<Watcher<T>> *list, const char *name, int id) {
  Watcher<T> *watcher = getWatcherByObservableName(list, name);
  if (watcher == nullptr) {
    st_log_warning(_HOOKS_MANAGER_TAG,
                   "Can't find watcher for observable %s", name);
    return nullptr;
  }
  Hook<T> *hook = watcher->getHookById(id);
  if (hook == nullptr) {
    st_log_warning(_HOOKS_MANAGER_TAG,
                   "Can't find hook id=%d for observable [%s]", id, name);
    return nullptr;
  }
  return hook;
}

template <typename T>
bool HooksManagerClass::deleteHookFromList(List<Watcher<T>> *list,
                                                   const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of observable is missing!");
    return false;
  }
  st_log_warning(_HOOKS_MANAGER_TAG,
               "Trying to delete observable [%s]'s hook id=%d", name, id);
  Watcher<T> *watcher = getWatcherByObservableName(list, name);
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
  if (!list->remove(watcher)) {
    return false;
  }
  delete watcher;
  st_log_warning(_HOOKS_MANAGER_TAG, "Watcher for observable [%s] removed!",
                 name);
  return true;
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcher(
    List<Watcher<T>> *list, const ObservableObject<T> *observable) {
  return list->findValue([observable](Watcher<T> *current) {
    return current->getObservable() == observable;
  });
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherByObservableName(
    List<Watcher<T>> *list, const char *name) {
  return list->findValue([name](Watcher<T> *current) {
    return strcmp(current->getObservable()->name, name) == 0;
  });
}

void HooksManagerClass::check() {
  #if ENABLE_SENSORS 
  if (_sensorsWatchers.size() > 0) {
    checkWatchers<SENSOR_DATA_TYPE>(&_sensorsWatchers);
  }
  #endif
  #if ENABLE_STATES
  if (_statesWatchers.size() > 0) {
    checkWatchers<STATE_DATA_TYPE>(&_statesWatchers);
  }
  #endif
}

template <typename T>
void HooksManagerClass::checkWatchers(List<Watcher<T>> *list) {
  list->forEach([](Watcher<T> *current) { current->check(); });
}

boolean HooksManagerClass::callHook(const char * type, const char * name, int id, String value) {
  if (strlen(type) == 0 || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Empty type or name!");
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
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return callWatcherHook<SENSOR_DATA_TYPE>(&_sensorsWatchers, name, id, emptyValue ? 0 : value.toInt(), emptyValue);
  }
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return callWatcherHook<STATE_DATA_TYPE>(&_statesWatchers, name, id, value, emptyValue);
  }
  #endif
  st_log_error(_HOOKS_MANAGER_TAG, "Type %s not supported!", type);
  return false;
}

template <typename T>
boolean HooksManagerClass::callWatcherHook(List<Watcher<T>>* list, const char * name, int id, T value, boolean emptyValue) {
  Watcher<T> * watcher = getWatcherByObservableName(list, name);
  if (watcher == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Can't find watcher for observable with name=%s", name);
    return false;
  }
  Hook<T> * hook = watcher->getHookById(id);
  if (hook == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Can't find hook for observable %s by id=%d", name, id);
    return false;
  }
  const ObservableObject<T> * obs = watcher->getObservable();
  if (obs == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "OBSERVABLE NULLPTR! HOW???");
    return false;
  }
  if (emptyValue) {
    st_log_info(_HOOKS_MANAGER_TAG, "Extracting value and calling hook");
    T v = obs->valueProvider();
    hook->call(v);
  } else {
    st_log_info(_HOOKS_MANAGER_TAG, "Calling hook with provided value");
    hook->call(value);
  }
  return true;
}


int indexOf(char target, const char * data, int length = -1) {
  if (data == nullptr || strlen(data) == 0) {
    return -1;
  }

  if (length = -1) {
    length = strlen(data);
  }

  for (int i = 0; i < length; i++) {
    if (data[i] == target) {
      return i;
    }
  }

  return -1;
}

void HooksManagerClass::loadFromSettings() {
  String fromSettings = SettingsRepository.getHooks();

  if (fromSettings.isEmpty()) {
    st_log_info(_HOOKS_MANAGER_TAG, "No hooks in settings");
    return;
  }

  bool failedBuild = false;
  int address = 0, tmp = 0;
  int dataLength = fromSettings.length();
  const char * data = fromSettings.c_str();

  st_log_debug(_HOOKS_MANAGER_TAG, "Building hooks from settings");
  while(address < dataLength) {
    ObservableType type = static_cast<ObservableType>(data[address] - '0');
    String name;

    for (address = address + 1; address < dataLength; address++) {
      if (data[address] == '\t') {
        break;
      }
      name += data[address];
    }

    address++;
    if (type == OBS_SENSOR) {
      failedBuild = loadHooks<SENSOR_DATA_TYPE>(ObservablesManager.getObservableObject<SENSOR_DATA_TYPE>(name.c_str()), data, &address, dataLength) || failedBuild;
    } else if (type == OBS_STATE) {
      failedBuild = loadHooks<STATE_DATA_TYPE>(ObservablesManager.getObservableObject<STATE_DATA_TYPE>(name.c_str()), data, &address, dataLength) || failedBuild;
    } else {
      st_log_error(_HOOKS_MANAGER_TAG, "Unkonwn observable hook type: %d", type);
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
bool HooksManagerClass::loadHooks(const ObservableObject<T> * observable, const char * data, int * address, int length) {
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
  _statesWatchers.forEach([&](Watcher<STATE_DATA_TYPE> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    data += watcher->toString();
    data += "\n";
  });
  #endif
  #if ENABLE_SENSORS 
  _sensorsWatchers.forEach([&](Watcher<SENSOR_DATA_TYPE> *watcher) {
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
  #if ENABLE_SENSORS 
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return getObservableHooksJsonFromList<SENSOR_DATA_TYPE>(&_sensorsWatchers, name);
  } 
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return getObservableHooksJsonFromList<STATE_DATA_TYPE>(&_statesWatchers, name);
  }
  #endif
  st_log_error(_HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  JsonDocument doc;
  return doc;
}

template <typename T>
JsonDocument HooksManagerClass::getObservableHooksJsonFromList(List<Watcher<T>> *list, const char *name) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of observable is missing!");
  } else {
    Watcher<T> *watcher = getWatcherByObservableName(list, name);
    if (watcher != nullptr) {
      return watcher->getObservableHooksJson();
    } else {
      st_log_warning(_HOOKS_MANAGER_TAG,
                     "Can't find watcher for observable [%s]", name);
    }
  }
  JsonDocument doc;
  return doc;
}

#endif