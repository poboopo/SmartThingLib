#include "hooks/HooksManager.h"

#if ENABLE_HOOKS

#include <type_traits>

#include "sensors/SensorsManager.h"
#include "hooks/builders/HooksBuilder.h"
#include "hooks/watcher/Watcher.h"
#include "settings/SettingsRepository.h"

#if ENABLE_LOGGER
  const char * const _HOOKS_MANAGER_TAG = "hooks_manager";

  const char * const _errorSensorNameMissing = "Sensor name is missing";
  const char * const _errorSensorObjectMissing = "Sensor object is missing";
  const char * const _errorNoSuchSensor = "No such sensor";
#endif

HooksManagerClass HooksManager;

int HooksManagerClass::addHook(const char * sensorName, const char * data) {
  if (sensorName == nullptr || strlen(sensorName) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorNameMissing);
    return -1;
  }

  #if ENABLE_TEXT_SENSORS
  const Sensor<TEXT_SENSOR_TYPE> * textSensor = SensorsManager.getSensor<TEXT_SENSOR_TYPE>(sensorName);
  if (textSensor != nullptr) {
    return HooksManager.addHook<TEXT_SENSOR_TYPE>(textSensor, data);
  }
  #endif
  #if ENABLE_NUMBER_SENSORS 
  const Sensor<NUMBER_SENSOR_TYPE> * numberSensor = SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(sensorName);
  if (numberSensor != nullptr) {
    return HooksManager.addHook<NUMBER_SENSOR_TYPE>(numberSensor, data);
  }
  #endif
  
  st_log_error(_HOOKS_MANAGER_TAG, "This sensor type is not supported");
  return -1;
}

template<typename T>
int HooksManagerClass::addHook(const Sensor<T> * sensor, const char * data) {
  if (sensor == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorObjectMissing);
    return -1;
  }

  if (data == nullptr || strlen(data) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook's data string can't be empty");
    return -1;
  }

  st_log_info(_HOOKS_MANAGER_TAG, "Trying to build hook for %s", sensor->name());
  st_log_debug(_HOOKS_MANAGER_TAG, "Hook string: %s", data);
  
  int id = -1;
  Hook<T> * hook = HooksBuilder::build<T>(data);
  if (hook != nullptr) {
    id = addHook<T>(sensor, hook);
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
int HooksManagerClass::addHook(const Sensor<T> *sensor, Hook<T> *hook) {
  if (sensor == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorObjectMissing);
    return -1;
  }
  if (hook == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook is missing, skipping...");
    return -1;
  }

  Watcher<T> *watcher = getWatcherOrCreate<T>(sensor);
  if (watcher == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Failed to get watcher!");
    return -1;
  }

  if (!watcher->addHook(hook)) {
    st_log_error(_HOOKS_MANAGER_TAG, "Failed to add hook in watcher");
    return -1;
  }
  _hooksCount++;
  st_log_info(_HOOKS_MANAGER_TAG, "Added new hook(id=%d) for sensor %s", hook->getId(), sensor->name());

  return hook->getId();
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherOrCreate(const Sensor<T> *sensor) {
  if (sensor == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorObjectMissing);
    return nullptr;
  }

  Watcher<T> *watcher = getWatcher<T>(sensor);
  if (watcher == nullptr) {
    st_log_debug(_HOOKS_MANAGER_TAG, "Creating new watcher for sensor %s", sensor->name());
    watcher = new Watcher<T>(sensor);
    if (getWatchersList<T>()->append(watcher) < 0) {
      st_log_error(_HOOKS_MANAGER_TAG, "Failed to append new watcher in list for %s", sensor->name());
      delete watcher;
      return nullptr;
    }
    st_log_debug(_HOOKS_MANAGER_TAG, "Added new watcher for sensor %s", sensor->name());
  }
  return watcher;
}

bool HooksManagerClass::deleteHook(const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorNameMissing);
    return -1;
  }

  #if ENABLE_NUMBER_SENSORS
  if (SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(name) != nullptr) {
    return deleteHook<NUMBER_SENSOR_TYPE>(name, id);
  } 
  #endif
  #if ENABLE_TEXT_SENSORS
  if (SensorsManager.getSensor<TEXT_SENSOR_TYPE>(name) != nullptr) {
    return deleteHook<TEXT_SENSOR_TYPE>(name, id);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "No such sensor");
  return -1;
}

template <typename T>
bool HooksManagerClass::deleteHook(const char *name, int id) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of sensor is missing!");
    return false;
  }

  st_log_warning(_HOOKS_MANAGER_TAG, "Trying to delete sensor [%s]'s hook id=%d", name, id);
  Watcher<T> *watcher = getWatcherBySensorName<T>(name);
  if (watcher == nullptr || !watcher->removeHook(id)) {
    return false;
  }

  _hooksCount--;
  st_log_warning(_HOOKS_MANAGER_TAG,
                 "Hook № %d of sensor [%s] was deleted", id, name);
  if (watcher->haveHooks()) {
    return true;
  }
  st_log_debug(_HOOKS_MANAGER_TAG,
               "No hooks left for sensor [%s], removing watcher!",
               name);
  if (!getWatchersList<T>()->remove(watcher)) {
    return false;
  }
  delete watcher;
  st_log_warning(_HOOKS_MANAGER_TAG, "Watcher for sensor [%s] removed!",
                 name);
  return true;
}

bool HooksManagerClass::updateHook(JsonDocument doc) {
  const char * sensor = doc["sensor"];
  if (sensor == nullptr || strlen(sensor) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Sensor name is missing!");
    return false;
  }

  JsonDocument hookObject = doc["hook"];
  if (hookObject.size() == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook object is missing!");
    return false;
  }
  if (!hookObject[_idHookField].is<JsonVariant>()) {
    st_log_error(_HOOKS_MANAGER_TAG, "Hook id property is missing!");
    return false;
  }


  #if ENABLE_NUMBER_SENSORS
    if (SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(sensor)) {
      return updateHook<NUMBER_SENSOR_TYPE>(sensor, hookObject);
    }
  #endif
  #if ENABLE_TEXT_SENSORS
    if (SensorsManager.getSensor<TEXT_SENSOR_TYPE>(sensor)) {
      return updateHook<TEXT_SENSOR_TYPE>(sensor, hookObject);
    }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "No such sensor");
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
                 "Hook id=%d for sensor [%s] is readonly!", id, name);
    return false;
  }

  st_log_info(_HOOKS_MANAGER_TAG, "Trying to update hook id=%d for sensor [%s]", id, name);

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

  st_log_info(_HOOKS_MANAGER_TAG, "Hook id=%d for sensor [%s] was updated!", id, name);
  return true;
}

template <typename T>
Hook<T> *HooksManagerClass::getHookFromWatcher(const char *name, int id) {
  Watcher<T> *watcher = getWatcherBySensorName<T>(name);
  if (watcher == nullptr) {
    return nullptr;
  }
  Hook<T> *hook = watcher->getHookById(id);
  if (hook == nullptr) {
    st_log_warning(_HOOKS_MANAGER_TAG, "Can't find hook id=%d for sensor [%s]", id, name);
    return nullptr;
  }
  return hook;
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcher(const Sensor<T> *sensor) {
  return getWatchersList<T>()->findValue([sensor](Watcher<T> *current) {
    return current->getSensor() == sensor;
  });
}

template <typename T>
Watcher<T> *HooksManagerClass::getWatcherBySensorName(const char *name) {
  return getWatchersList<T>()->findValue([name](Watcher<T> *current) {
    return strcmp(current->getSensor()->name(), name) == 0;
  });
}

void HooksManagerClass::check() {
  #if ENABLE_NUMBER_SENSORS 
  if (_sensorsWatchers.size() > 0) {
    checkWatchers<NUMBER_SENSOR_TYPE>();
  }
  #endif
  #if ENABLE_TEXT_SENSORS
  if (_statesWatchers.size() > 0) {
    checkWatchers<TEXT_SENSOR_TYPE>();
  }
  #endif
}

template <typename T>
void HooksManagerClass::checkWatchers() {
  getWatchersList<T>()->forEach([](Watcher<T> *current) { current->check(); });
}

boolean HooksManagerClass::callHook(const char * name, int id, String value) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorNameMissing);
    return false;
  }

  boolean emptyValue = value.isEmpty();
  st_log_info(
    _HOOKS_MANAGER_TAG,
    "Trying to call hook sensor_name=%s, id=%d, value=%s",
    name,
    id,
    emptyValue ? "" : value.c_str()
  );

  #if ENABLE_NUMBER_SENSORS
  if (SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(name)) {
    return callWatcherHook<NUMBER_SENSOR_TYPE>(name, id, emptyValue ? 0 : value.toInt(), emptyValue);
  }
  #endif
  #if ENABLE_TEXT_SENSORS
  if (SensorsManager.getSensor<TEXT_SENSOR_TYPE>(name)) {
    return callWatcherHook<TEXT_SENSOR_TYPE>(name, id, value, emptyValue);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "No such sensor");
  return false;
}

template <typename T>
boolean HooksManagerClass::callWatcherHook(const char * name, int id, T value, boolean emptyValue) {
  Watcher<T> * watcher = getWatcherBySensorName<T>(name);
  if (watcher == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Can't find watcher for sensor with name=%s", name);
    return false;
  }
  Hook<T> * hook = watcher->getHookById(id);
  if (hook == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "Can't find hook for sensor %s by id=%d", name, id);
    return false;
  }
  const Sensor<T> * sensor = watcher->getSensor();
  if (sensor == nullptr) {
    st_log_error(_HOOKS_MANAGER_TAG, "SENSOR NULLPTR! HOW???");
    return false;
  }
  if (emptyValue) {
    st_log_info(_HOOKS_MANAGER_TAG, "Extracting value and calling hook");
    T v = sensor->provideValue();
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
    String name;

    for (; address < dataLength; address++) {
      if (data[address] == '\t') {
        break;
      }
      name += data[address];
    }

    address++;

    bool sensorFound = false;
    #if ENABLE_NUMBER_SENSORS
    const Sensor<NUMBER_SENSOR_TYPE> * numberSensor = SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(name.c_str());
    if (numberSensor != nullptr) {
      sensorFound = true;
      failedBuild = loadHooks<NUMBER_SENSOR_TYPE>(numberSensor, data, &address, dataLength) || failedBuild;
    }
    #endif
    #if ENABLE_TEXT_SENSORS
    const Sensor<TEXT_SENSOR_TYPE> * textSensor = SensorsManager.getSensor<TEXT_SENSOR_TYPE>(name.c_str());
    if (textSensor != nullptr) {
      sensorFound = true;
      failedBuild = loadHooks<TEXT_SENSOR_TYPE>(textSensor, data, &address, dataLength) || failedBuild;
    }
    #endif

    if (!sensorFound) {
      st_log_error(_HOOKS_MANAGER_TAG, "Can't find sensor %s", name.c_str());
      st_log_error(_HOOKS_MANAGER_TAG, "FAILED TO LOAD HOOKS FROM SETTINGS");
      return;
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
bool HooksManagerClass::loadHooks(const Sensor<T> * sensor, const char * data, int * address, int length) {
  bool res = false;
  String buff; // todo remove, pass hook length

  for (; (*address) < length; (*address)++) {
    if (data[(*address)] == '\t' || data[(*address)] == '\n') {
      res = addHook(sensor, buff.c_str()) == -1 || res;
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

  #if ENABLE_TEXT_SENSORS
  _statesWatchers.forEach([&](Watcher<TEXT_SENSOR_TYPE> *watcher) {
    if (watcher == nullptr) {
      return;
    }
    data += watcher->toString();
    data += "\n";
  });
  #endif
  #if ENABLE_NUMBER_SENSORS 
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

JsonDocument HooksManagerClass::getSensorHooksJson(const char *name) {
  JsonDocument doc;

  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, _errorSensorNameMissing);
    doc["error"] = _errorSensorNameMissing;
    return doc;
  }

  #if ENABLE_NUMBER_SENSORS 
  if (SensorsManager.getSensor<NUMBER_SENSOR_TYPE>(name) != nullptr) {
    return getSensorHooksJson<NUMBER_SENSOR_TYPE>(name);
  } 
  #endif
  #if ENABLE_TEXT_SENSORS
  if (SensorsManager.getSensor<TEXT_SENSOR_TYPE>(name) != nullptr) {
    return getSensorHooksJson<TEXT_SENSOR_TYPE>(name);
  }
  #endif

  st_log_error(_HOOKS_MANAGER_TAG, "No such sensor");
  return doc;
}

template <typename T>
JsonDocument HooksManagerClass::getSensorHooksJson(const char *name) {
  if (name == nullptr || strlen(name) == 0) {
    st_log_error(_HOOKS_MANAGER_TAG, "Name of sensor is missing!");
  } else {
    Watcher<T> *watcher = getWatcherBySensorName<T>(name);
    if (watcher != nullptr) {
      return watcher->getSensorHooksJson();
    }
  }
  JsonDocument doc;
  return doc;
}

#if ENABLE_TEXT_SENSORS
  template <>
  List<Watcher<TEXT_SENSOR_TYPE>> *HooksManagerClass::getWatchersList() {
    return &_statesWatchers;
  }
#endif

#if ENABLE_NUMBER_SENSORS
  template <>
  List<Watcher<NUMBER_SENSOR_TYPE>> *HooksManagerClass::getWatchersList() {
    return &_sensorsWatchers;
  }
#endif

#endif