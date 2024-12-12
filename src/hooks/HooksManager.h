#ifndef HOOKS_MANAGER_H
#define HOOKS_MANAGER_H

#include "Features.h"
#if ENABLE_HOOKS 

#include <ArduinoJson.h>
#include <functional>

#include "hooks/impls/Hook.h"
#include "hooks/impls/LambdaHook.h"
#include "hooks/watcher/Watcher.h"
#include "sensors/Sensor.h"
#include "utils/List.h"

class HooksManagerClass {
 public:
  void loadFromSettings();

  int addHook(SensorType observableType, const char * observableName, const char * data);

  bool deleteHook(const char* type, const char* name, int id);
  bool updateHook(JsonDocument doc);

  void check();
  boolean callHook(const char * type, const char * name, int id, String value);

  JsonDocument getObservableHooksJson(const char* type, const char* name);

  bool saveInSettings();

  int16_t getTotalHooksCount() { return _hooksCount; }

 private:
  #if ENABLE_SENSORS 
  List<Watcher<NUMBER_SENSOR_TYPE>> _sensorsWatchers;
  #endif

  #if ENABLE_STATES
  List<Watcher<TEXT_SENSOR_TYPE>> _statesWatchers;
  #endif

  int _hooksCount = 0;

  template<typename T>
  bool loadHooks(const Sensor<T> * observable, const char * data, int * address, int length);

  template<typename T>
  int addHook(const Sensor<T> * observable, const char * data);

  template <typename T>
  int addHook(const Sensor<T>* obj, Hook<T>* hook);

  template <typename T>
  Watcher<T>* getWatcher(const Sensor<T>* observable);

  template <typename T>
  Watcher<T>* getWatcherByObservableName(const char* name);

  template <typename T>
  Hook<T>* getHookFromWatcher(const char* name, int id);

  template <typename T>
  JsonDocument getObservableHooksJson(const char* name);

  template <typename T>
  bool deleteHook(const char* name, int id);

  template <typename T>
  bool updateHook(const char* name, JsonDocument &hookObject);

  template <typename T>
  void checkWatchers();

  template <typename T>
  boolean callWatcherHook(const char * name, int id, T value, boolean emptyValue);

  template <typename T>
  Watcher<T>* getWatcherOrCreate(
      const Sensor<T>* obj);

  template <typename T>
  List<Watcher<T>>* getWatchersList();
};

extern HooksManagerClass HooksManager;

#endif

#endif