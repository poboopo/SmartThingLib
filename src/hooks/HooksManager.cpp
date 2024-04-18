#include "hooks/HooksManager.h"

#include "SmartThing.h"
#include "hooks/builders/HooksFactory.h"
#include "hooks/watchers/DeviceStateWatcher.h"
#include "hooks/watchers/SensorWatcher.h"
#include "settings/SettingsManager.h"

#define HOOKS_MANAGER_TAG "hooks_manager"

Hook::HooksManagerClass HooksManager;

namespace Hook {
using namespace Configurable::Sensor;
using namespace Configurable::DeviceState;
using Configurable::ConfigurableObject;

void HooksManagerClass::loadFromSettings() {
  JsonArray hooksInfo = STSettings.getHooks();
  if (hooksInfo.size() == 0) {
    LOGGER.debug(HOOKS_MANAGER_TAG, "There is not hooks in settings");
    return;
  }

  for (int i = 0; i < hooksInfo.size(); i++) {
    JsonObject observable = hooksInfo[i]["observable"];
    JsonArray hooks = hooksInfo[i]["hooks"];
    for (int j = 0; j < hooks.size(); j++) {
      createHookFromJson(observable, hooks[j]);
    }
  }
}

int HooksManagerClass::createHookFromJson(const char *json) {
  if (json == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Json is null!");
    return -1;
  }
  LOGGER.debug(HOOKS_MANAGER_TAG, "Creating hook from json: %s", json);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);

  return createHookFromJson(doc["observable"], doc["hook"]);
}

int HooksManagerClass::createHookFromJson(JsonObject observableInfo,
                                                  JsonObject hook) {
  if (observableInfo.size() == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG,
                 "ObservableInfo object can't be empty!");
    return -1;
  }
  if (hook.size() == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Hook object can't be empty!");
    return -1;
  }

  const char *type = observableInfo["type"];
  const char *name = observableInfo["name"];
  if (type == nullptr || name == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG,
                 "Parameters observable type or name are missing!");
    return -1;
  }

  if (strcmp(type, STATE_TYPE) == 0) {
    return addHook<String>(SmartThing.getDeviceState(name),
                               HooksFactory::build<StateHook, String>(hook));
  } else if (strcmp(type, SENSOR_TYPE) == 0) {
    return addHook<int16_t>(SmartThing.getSensor(name),
                                HooksFactory::build<SensorHook, int16_t>(hook));
  }

  LOGGER.error(HOOKS_MANAGER_TAG, "Unkown observable object type: %s",
               type);
  return -1;
}

int HooksManagerClass::addHook(const Configurable::Sensor::Sensor * sensor, Hook<int16_t> * hook) {
  return addHook<int16_t>(sensor, hook);
}
int HooksManagerClass::addHook(const Configurable::DeviceState::DeviceState * state, Hook<String> * hook) {
  return addHook<String>(state, hook);
}

template <class T>
int HooksManagerClass::addHook(const ConfigurableObject<T> *obj,
                                       Hook<T> *hook) {
  if (obj == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG,
                 "Configurable object is missing, skipping...");
    return -1;
  }
  if (hook == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Hook is missing, skipping...");
    return -1;
  }

  Watcher<T> *watcher = getWatcherOrCreate<T>(obj);
  if (watcher == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Failed to get watcher!");
    return -1;
  }

  if (!watcher->addHook(hook)) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Failed to add hook in watcher");
    return -1;
  }
  _hooksCount++;
  LOGGER.info(HOOKS_MANAGER_TAG, "Added new hook(id=%d) for %s [%s]",
              hook->getId(), obj->type, obj->name);
  return hook->getId();
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherOrCreate(
    const ConfigurableObject<T> *obj) {
  if (obj == nullptr || obj->type == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Configurable object is missing");
    return nullptr;
  }

  List<Watcher<T>> *watchersList = getWatchersList<T>();
  if (watchersList == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Failed to define watchers list");
    return nullptr;
  }

  Watcher<T> *watcher = getWatcher<T>(watchersList, obj);
  if (watcher == nullptr) {
    LOGGER.debug(HOOKS_MANAGER_TAG, "Creating new watcher for %s [%s]",
                 obj->type, obj->name);
    watcher = createWatcher<T>(obj);
    if (watchersList->append(watcher) < 0) {
      LOGGER.error(HOOKS_MANAGER_TAG,
                   "Failed to append new watcher in list for %s [%s]",
                   obj->type, obj->name);
      delete watcher;
      return nullptr;
    }
    LOGGER.info(HOOKS_MANAGER_TAG, "Added new watcher for %s [%s]",
                obj->type, obj->name);
  } else {
    LOGGER.debug(HOOKS_MANAGER_TAG, "Watcher for %s [%s] already exists!",
                 obj->type, obj->name);
  }
  return watcher;
}

template <>
Watcher<int16_t> *HooksManagerClass::createWatcher(
    const ConfigurableObject<int16_t> *obj) {
  return new SensorWatcher((Sensor *)obj);
}

template <>
Watcher<String> *HooksManagerClass::createWatcher(
    const ConfigurableObject<String> *obj) {
  return new DeviceStateWatcher((DeviceState *)obj);
}

template <>
List<Watcher<int16_t>> *HooksManagerClass::getWatchersList() {
  return &_sensorsWatchers;
}

template <>
List<Watcher<String>> *HooksManagerClass::getWatchersList() {
  return &_statesWatchers;
}

bool HooksManagerClass::deleteHook(const char *type, const char *name,
                                           int id) {
  if (type == nullptr || strlen(type) == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Type of observable is missing!");
    return -1;
  }

  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return deleteHookFromList<int16_t>(&_sensorsWatchers, name, id);
  } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return deleteHookFromList<String>(&_statesWatchers, name, id);
  }

  LOGGER.error(HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  return -1;
}

bool HooksManagerClass::updateHook(DynamicJsonDocument doc) {
  JsonObject observable = doc["observable"];
  JsonObject hookObject = doc["hook"];

  if (observable.size() == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Observable object is missing!");
    return false;
  }
  if (hookObject.size() == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Hook object is missing!");
    return false;
  }
  if (!hookObject.containsKey("id")) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Hook id property is missing!");
    return false;
  }

  const char *name = observable["name"];
  const char *type = observable["type"];
  if (name == nullptr || type == nullptr) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Observable name or type is missing!");
    return false;
  }

  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return updateHook<int16_t>(&_sensorsWatchers, name, hookObject);
  } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return updateHook<String>(&_statesWatchers, name, hookObject);
  }
  LOGGER.error(HOOKS_MANAGER_TAG, "Observable type [%s] not supported!",
               type);
  return false;
}

// todo more logs with info?
template <typename T>
bool HooksManagerClass::updateHook(List<Watcher<T>> *list,
                                           const char *name,
                                           JsonObject hookObject) {
  if (!hookObject.containsKey("id")) {
    LOGGER.error(HOOKS_MANAGER_TAG,
                 "Id value in hook object is missing!");
    return false;
  }

  int id = hookObject["id"];
  Hook<T> *hook = getHookFromWatcherList(list, name, id);
  if (hook == nullptr) {
    return false;
  }
  if (hook->isReadonly()) {
    LOGGER.error(HOOKS_MANAGER_TAG,
                 "Hook %d for observable [%s] is readonly!", id, name);
    return false;
  }

  if (hookObject.containsKey("trigger")) {
    String trigger = hookObject["trigger"].as<String>();
    if (trigger.isEmpty() || trigger.equals("null")) {
      hook->disableTrigger();
      LOGGER.debug(HOOKS_MANAGER_TAG, "Trigger disabled");
    } else {
      hook->enableTrigger();
      hook->setTriggerValue(hookObject["trigger"]);
      LOGGER.debug(HOOKS_MANAGER_TAG, "New triggeValuer=%s", trigger.c_str());
    }
  }

  if (hookObject.containsKey("compareType")) {
    String compareType = hookObject["compareType"].as<String>();
    hook->setCompareType(compareType);
    LOGGER.debug(HOOKS_MANAGER_TAG, "New compareType=%s", compareType.c_str());
  }

  hook->updateCustom(hookObject);
  updateTypeSpecificHookValues(hook, hookObject);

  LOGGER.info(HOOKS_MANAGER_TAG,
              "Hook id=%d for observable [%s] was updated!", id, name);
  return true;
}

template<>
void HooksManagerClass::updateTypeSpecificHookValues(Hook<int16_t> * hook, JsonObject hookObject) {
  if (hookObject.containsKey("threshold")) {
    int16_t threshold = hookObject["threshold"];
    ((SensorHook *) hook)->setThreshold(threshold);
    LOGGER.debug(HOOKS_MANAGER_TAG, "New threshold=%d", threshold);
  }
}

template<>
void HooksManagerClass::updateTypeSpecificHookValues(Hook<String> * hook, JsonObject hookObject) {
}

template <typename T>
Hook<T> *HooksManagerClass::getHookFromWatcherList(
    List<Watcher<T>> *list, const char *name, int id) {
  Watcher<T> *watcher = getWatcherByObservableName(list, name);
  if (watcher == nullptr) {
    LOGGER.warning(HOOKS_MANAGER_TAG,
                   "Can't find watcher for observable %s", name);
    return nullptr;
  }
  Hook<T> *hook = watcher->getHookById(id);
  if (hook == nullptr) {
    LOGGER.warning(HOOKS_MANAGER_TAG,
                   "Can't find hook id=%d for observable [%s]", id, name);
    return nullptr;
  }
  return hook;
}

template <typename T>
bool HooksManagerClass::deleteHookFromList(List<Watcher<T>> *list,
                                                   const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Name of observable is missing!");
    return false;
  }
  LOGGER.debug(HOOKS_MANAGER_TAG,
               "Trying to delete observable [%s]'s hook id=%d", name, id);
  Watcher<T> *watcher = getWatcherByObservableName(list, name);
  if (watcher == nullptr || !watcher->removeHook(id)) {
    return false;
  }
  _hooksCount--;
  LOGGER.warning(HOOKS_MANAGER_TAG,
                 "Hook № %d of observable [%s] was deleted", id, name);
  if (watcher->haveHooks()) {
    return true;
  }
  LOGGER.debug(HOOKS_MANAGER_TAG,
               "No hooks left for observable [%s], removing watcher!",
               name);
  if (!list->remove(watcher)) {
    return false;
  }
  delete watcher;
  LOGGER.warning(HOOKS_MANAGER_TAG, "Watcher for observable [%s] removed!",
                 name);
  return true;
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcher(
    List<Watcher<T>> *list, const ConfigurableObject<T> *observable) {
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
  if (_sensorsWatchers.size() > 0) {
    checkWatchers<int16_t>(&_sensorsWatchers);
  }
  if (_statesWatchers.size() > 0) {
    checkWatchers<String>(&_statesWatchers);
  }
}

template <typename T>
void HooksManagerClass::checkWatchers(List<Watcher<T>> *list) {
  list->forEach([](Watcher<T> *current) { current->check(); });
}

void HooksManagerClass::saveHooksToSettings() {
  LOGGER.debug(HOOKS_MANAGER_TAG, "Saving hooks");
  STSettings.setHooks(allHooksToJson(true, true).as<JsonArray>());
  STSettings.save();
  LOGGER.debug(HOOKS_MANAGER_TAG, "Hooks were saved");
}

DynamicJsonDocument HooksManagerClass::allHooksToJson(
    bool ignoreReadOnly, bool shortJson) {
  if (_hooksCount == 0) {
    LOGGER.debug(HOOKS_MANAGER_TAG, "No hooks, creating empty doc");
    DynamicJsonDocument doc(0);
    doc.to<JsonArray>();
    return doc;
  }

  int size = _hooksCount * HOOK_INFO_DOC_SIZE +
             (_statesWatchers.size() + _sensorsWatchers.size()) *
                 WATCHER_INFO_DOC_SIZE;

  if (size == 0) {
    LOGGER.debug(HOOKS_MANAGER_TAG,
                 "DynamicJsonDocument size = 0, creating empty doc");
    DynamicJsonDocument doc(0);
    return doc;
  }

  LOGGER.debug(HOOKS_MANAGER_TAG, "DynamicJsonDoc size for hooks = %d",
               size);
  DynamicJsonDocument doc(size);
  _sensorsWatchers.forEach([&](Watcher<int16_t> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    DynamicJsonDocument wjs = watcher->toJson(ignoreReadOnly, shortJson);
    if (wjs.size() > 0) doc.add(wjs);
  });
  _statesWatchers.forEach([&](Watcher<String> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    DynamicJsonDocument wjs = watcher->toJson(ignoreReadOnly, shortJson);
    if (wjs.size() > 0) doc.add(wjs);
  });
  return doc;
}

DynamicJsonDocument HooksManagerClass::getWatchersInfo() {
  DynamicJsonDocument doc(1024);
  if (_sensorsWatchers.size() > 0) {
    LOGGER.debug(HOOKS_MANAGER_TAG,
                 "Collecting info from sensors watchers");
    JsonArray array = doc.createNestedArray(SENSOR_WATCHER_TYPE);
    collectInfo<int16_t>(&_sensorsWatchers, &array);
  }
  if (_statesWatchers.size() > 0) {
    LOGGER.debug(HOOKS_MANAGER_TAG,
                 "Collecting info from device state watchers");
    JsonArray array = doc.createNestedArray(STATE_WATCHER_TYPE);
    collectInfo<String>(&_statesWatchers, &array);
  }
  return doc;
}

DynamicJsonDocument HooksManagerClass::getObservableHooksJson(
    const char *type, const char *name) {
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return getObservableHooksJsonFromList<int16_t>(&_sensorsWatchers, name);
  } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return getObservableHooksJsonFromList<String>(&_statesWatchers, name);
  }
  LOGGER.error(HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  DynamicJsonDocument doc(4);
  return doc;
}

template <typename T>
DynamicJsonDocument HooksManagerClass::getObservableHooksJsonFromList(
    List<Watcher<T>> *list, const char *name) {
  if (name == nullptr || strlen(name) == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Name of observable is missing!");
  } else {
    Watcher<T> *watcher = getWatcherByObservableName(list, name);
    if (watcher != nullptr) {
      return watcher->getObservableHooksJson();
    } else {
      LOGGER.warning(HOOKS_MANAGER_TAG,
                     "Can't find watcher for observable [%s]", name);
    }
  }
  DynamicJsonDocument doc(4);
  return doc;
}

DynamicJsonDocument HooksManagerClass::getHookJsonById(const char *type,
                                                               const char *name,
                                                               int id) {
  if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
    return getHookJsonFromList<int16_t>(&_sensorsWatchers, name, id);
  } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
    return getHookJsonFromList<String>(&_statesWatchers, name, id);
  }
  LOGGER.error(HOOKS_MANAGER_TAG, "Type [%s] not supported", type);
  DynamicJsonDocument doc(4);
  return doc;
}

template <typename T>
DynamicJsonDocument HooksManagerClass::getHookJsonFromList(
    List<Watcher<T>> *list, const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    LOGGER.error(HOOKS_MANAGER_TAG, "Name of observable is missing!");
  } else {
    Watcher<T> *watcher = getWatcherByObservableName(list, name);
    if (watcher != nullptr) {
      Hook<T> *hook = watcher->getHookById(id);
      if (hook != nullptr) {
        return hook->toJson(false);
      } else {
        LOGGER.warning(HOOKS_MANAGER_TAG,
                       "Can't find hook with id [%d] in watcher for [%s]",
                       id, name);
      }
    } else {
      LOGGER.warning(HOOKS_MANAGER_TAG,
                     "Can't find watcher for observable [%s]", name);
    }
  }
  DynamicJsonDocument doc(4);
  return doc;
}
}  // namespace Hook