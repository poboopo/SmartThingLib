#include "hooks/HooksManager.h"

#if ENABLE_HOOKS 

#include "observable/ObservablesManager.h"
#include "hooks/builders/HooksFactory.h"
#include "hooks/watchers/DeviceStateWatcher.h"
#include "hooks/watchers/SensorWatcher.h"
#include "settings/SettingsRepository.h"

static const char * _HOOKS_MANAGER_TAG = "hooks_manager";

HooksManagerClass HooksManager;

void HooksManagerClass::loadFromSettings() {
  JsonDocument hooksInfo = SettingsRepository.getHooks();
  if (hooksInfo.size() == 0) {
    st_log_debug(_HOOKS_MANAGER_TAG, "There is no hooks in settings");
    return;
  }

  bool needGhostDelete = false;
  for (unsigned int i = 0; i < hooksInfo.size(); i++) {
    JsonObject config = hooksInfo[i];
    JsonObject observable = config["observable"];
    JsonArray hooks = config["hooks"];
    for (unsigned int j = 0; j < hooks.size(); j++) {
      needGhostDelete = needGhostDelete || createHookFromJson(observable, hooks[j]) < 0;
    }
  }

  if (needGhostDelete) {
    st_log_warning(_HOOKS_MANAGER_TAG, "Have some ghost hooks to delete, trying to save correct hooks list");
    saveHooksToSettings();
    st_log_info(_HOOKS_MANAGER_TAG, "Ghost hooks removed");
  }
}

int HooksManagerClass::createHookFromJson(const char *json) {
  if (json == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Json is null!");
    return -1;
  }
  st_log_debug(_HOOKS_MANAGER_TAG, "Creating hook from json: %s", json);

  JsonDocument doc;
  deserializeJson(doc, json);

  return createHookFromJson(doc["observable"], doc["hook"]);
}

int HooksManagerClass::createHookFromJson(JsonObject observableInfo,
                                                  JsonObject hook) {
  if (observableInfo.size() == 0) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "ObservableInfo object can't be empty!");
    return -1;
  }
  if (hook.size() == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook object can't be empty!");
    return -1;
  }

  ObservableType type = observableTypeFromStr(observableInfo["type"]);
  const char *name = observableInfo["name"];
  if (type == UNKNOWN_OBS_TYPE || name == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG,
                 "Parameters observable type or name are missing!");
    return -1;
  }

  st_log_debug(_HOOKS_MANAGER_TAG, "Trying to build hook for [%u] %s", type, name);

  #if ENABLE_STATES
  if (type = OBS_STATE) {
    return addHook<String>(ObservablesManager.getDeviceState(name),
                               HooksFactory::build<StateHook, String>(hook));
  }
  #endif
  #if ENABLE_SENSORS 
  if (type == OBS_SENSOR) {
    return addHook<int16_t>(ObservablesManager.getSensor(name),
                                HooksFactory::build<SensorHook, int16_t>(hook));
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "Observable object type not supported: %u", type);
  return -1;
}

template <class T>
int HooksManagerClass::addHook(const ObservableObject<T> *obj,
                                       Hook<T> *hook) {
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
  st_log_info(_HOOKS_MANAGER_TAG, "Added new hook(id=%d) for %u [%s]",
              hook->getId(), obj->type, obj->name);
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
Watcher<String> *HooksManagerClass::createWatcher(
    const ObservableObject<String> *obj) {
  return new DeviceStateWatcher((DeviceState *)obj);
}

template <>
List<Watcher<String>> *HooksManagerClass::getWatchersList() {
  return &_statesWatchers;
}

int HooksManagerClass::addHook(const DeviceState * state, Hook<String> * hook) {
  return addHook<String>(state, hook);
}
#endif

#if ENABLE_SENSORS 
template <>
Watcher<int16_t> *HooksManagerClass::createWatcher(const ObservableObject<int16_t> *obj) {
  return new SensorWatcher((Sensor *)obj);
}

template <>
List<Watcher<int16_t>> *HooksManagerClass::getWatchersList() {
  return &_sensorsWatchers;
}

int HooksManagerClass::addHook(const Sensor * sensor, Hook<int16_t> * hook) {
  return addHook<int16_t>(sensor, hook);
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
    return deleteHookFromList<int16_t>(&_sensorsWatchers, name, id);
  } 
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return deleteHookFromList<String>(&_statesWatchers, name, id);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  return -1;
}

bool HooksManagerClass::updateHook(JsonDocument doc) {
  JsonObject observable = doc["observable"];
  JsonObject hookObject = doc["hook"];

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
    return updateHook<int16_t>(&_sensorsWatchers, name, hookObject);
  }
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return updateHook<String>(&_statesWatchers, name, hookObject);
  }
  #endif
  st_log_error(_HOOKS_MANAGER_TAG, "Observable type [%s] not supported!",
               type);
  return false;
}

template <typename T>
bool HooksManagerClass::updateHook(List<Watcher<T>> *list,
                                           const char *name,
                                           JsonObject hookObject) {
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

  HooksFactory::update<T>(hook, hookObject);
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

template <typename T>
void HooksManagerClass::collectInfo(List<Watcher<T>> *list,
                                        JsonArray *array) {
  list->forEach(
      [&](Watcher<T> *current) { array->add(current->getObservableInfo()); });
}

void HooksManagerClass::check() {
  #if ENABLE_SENSORS 
  if (_sensorsWatchers.size() > 0) {
    checkWatchers<int16_t>(&_sensorsWatchers);
  }
  #endif
  #if ENABLE_STATES
  if (_statesWatchers.size() > 0) {
    checkWatchers<String>(&_statesWatchers);
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
    return callWatcherHook<int16_t>(&_sensorsWatchers, name, id, emptyValue ? 0 : value.toInt(), emptyValue);
  }
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return callWatcherHook<String>(&_statesWatchers, name, id, value, emptyValue);
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

void HooksManagerClass::saveHooksToSettings() {
  st_log_debug(_HOOKS_MANAGER_TAG, "Saving hooks");
  SettingsRepository.setHooks(allHooksToJson(true, true));
}

JsonDocument HooksManagerClass::allHooksToJson(bool ignoreReadOnly, bool shortJson) {
  JsonDocument doc;
  doc.to<JsonArray>();
  if (_hooksCount == 0) {
    st_log_debug(_HOOKS_MANAGER_TAG, "No hooks, creating empty doc");
    return doc;
  }

  #if ENABLE_SENSORS 
  _sensorsWatchers.forEach([&](Watcher<int16_t> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    JsonDocument wjs = watcher->toJson(ignoreReadOnly, shortJson);
    if (!wjs.isNull()) doc.add(wjs);
  });
  #endif
  #if ENABLE_STATES
  _statesWatchers.forEach([&](Watcher<String> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    JsonDocument wjs = watcher->toJson(ignoreReadOnly, shortJson);
    if (!wjs.isNull()) doc.add(wjs);
  });
  #endif
  return doc;
}

JsonDocument HooksManagerClass::getWatchersInfo() {
  JsonDocument doc;
  #if ENABLE_SENSORS 
  if (_sensorsWatchers.size() > 0) {
    st_log_debug(_HOOKS_MANAGER_TAG,
                 "Collecting info from sensors watchers");
    JsonArray array = doc[SENSOR_WATCHER_TYPE].to<JsonArray>();
    collectInfo<int16_t>(&_sensorsWatchers, &array);
  }
  #endif
  #if ENABLE_STATES
  if (_statesWatchers.size() > 0) {
    st_log_debug(_HOOKS_MANAGER_TAG,
                 "Collecting info from device state watchers");
    JsonArray array = doc[STATE_WATCHER_TYPE].to<JsonArray>();
    collectInfo<String>(&_statesWatchers, &array);
  }
  #endif
  return doc;
}

JsonDocument HooksManagerClass::getObservableHooksJson(const char *type, const char *name) {
  #if ENABLE_SENSORS 
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return getObservableHooksJsonFromList<int16_t>(&_sensorsWatchers, name);
  } 
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return getObservableHooksJsonFromList<String>(&_statesWatchers, name);
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

JsonDocument HooksManagerClass::getHookJsonById(const char *type,
                                                               const char *name,
                                                               int id) {
  #if ENABLE_SENSORS 
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return getHookJsonFromList<int16_t>(&_sensorsWatchers, name, id);
  } 
  #endif
  #if ENABLE_STATES
  if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return getHookJsonFromList<String>(&_statesWatchers, name, id);
  }
  #endif
  st_log_error(_HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  JsonDocument doc;
  return doc;
}

template <typename T>
JsonDocument HooksManagerClass::getHookJsonFromList(
    List<Watcher<T>> *list, const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of observable is missing!");
  } else {
    Watcher<T> *watcher = getWatcherByObservableName(list, name);
    if (watcher != nullptr) {
      Hook<T> *hook = watcher->getHookById(id);
      if (hook != nullptr) {
        return hook->toJson(false);
      } else {
        st_log_warning(_HOOKS_MANAGER_TAG,
                       "Can't find hook with id [%d] in watcher for [%s]",
                       id, name);
      }
    } else {
      st_log_warning(_HOOKS_MANAGER_TAG,
                     "Can't find watcher for observable [%s]", name);
    }
  }
  JsonDocument doc;
  return doc;
}

#endif