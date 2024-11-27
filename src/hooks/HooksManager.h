#ifndef HOOKS_MANAGER_H
#define HOOKS_MANAGER_H

#include "Features.h"
#if ENABLE_HOOKS 

#include <ArduinoJson.h>

#include <functional>

#include "hooks/impls/Hook.h"
#include "hooks/impls/LambdaHook.h"
#include "hooks/watchers/DeviceStateWatcher.h"
#include "hooks/watchers/SensorWatcher.h"
#include "hooks/watchers/Watcher.h"
#include "observable/ObservableObjects.h"
#include "utils/List.h"

class HooksManagerClass {
 public:
  void loadFromSettings();

  int addHook(ObservableType observableType, const char * observableName, const char * data);

  bool deleteHook(const char* type, const char* name, int id);
  bool updateHook(JsonDocument doc);

  void check();
  boolean callHook(const char * type, const char * name, int id, String value);

  JsonDocument getObservableHooksJson(const char* type,
                                                 const char* name);

  bool saveInSettings();

  int16_t getTotalHooksCount() { return _hooksCount; }

 private:
  #if ENABLE_SENSORS 
  List<Watcher<SENSOR_DATA_TYPE>> _sensorsWatchers;
  #endif

  #if ENABLE_STATES
  List<Watcher<STATE_DATA_TYPE>> _statesWatchers;
  #endif

  int _hooksCount = 0;

  template<typename T>
  bool loadHooks(const ObservableObject<T> * observable, const char * data, int * address, int length);

  template<typename T>
  int addHook(const ObservableObject<T> * observable, const char * data);

  template <typename T>
  int addHook(const ObservableObject<T>* obj, Hook<T>* hook);

  template <typename T>
  Watcher<T>* getWatcher(List<Watcher<T>>* list,
                         const ObservableObject<T>* observable);

  template <typename T>
  Watcher<T>* getWatcherByObservableName(List<Watcher<T>>* list,
                                         const char* name);

  template <typename T>
  Hook<T>* getHookFromWatcherList(List<Watcher<T>>* list,
                                          const char* name, int id);

  template <typename T>
  JsonDocument getObservableHooksJsonFromList(List<Watcher<T>>* list,
                                                         const char* name);

  template <typename T>
  bool deleteHookFromList(List<Watcher<T>>* list, const char* name, int id);

  template <typename T>
  bool updateHook(List<Watcher<T>>* list, const char* name,
                      JsonDocument &hookObject);

  template <typename T>
  void checkWatchers(List<Watcher<T>>* list);

  template <typename T>
  boolean callWatcherHook(List<Watcher<T>>* list, const char * name, int id, T value, boolean emptyValue);

  template <typename T>
  Watcher<T>* getWatcherOrCreate(
      const ObservableObject<T>* obj);

  template <typename T>
  Watcher<T>* createWatcher(const ObservableObject<T>* obj);

  template <typename T>
  List<Watcher<T>>* getWatchersList();
};

extern HooksManagerClass HooksManager;

#endif

#endif