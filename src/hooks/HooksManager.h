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

namespace Hook {
class HooksManagerClass {
 public:
  void loadFromSettings();
  int createHookFromJson(const char* json);
  int createHookFromJson(JsonObject observableInfo,
                             JsonObject hookInfo);

  #if ENABLE_SENSORS
  int addHook(const Observable::Sensor::Sensor * sensor, Hook<int16_t> * hook);
  #endif
  #if ENABLE_STATES
  int addHook(const Observable::DeviceState::DeviceState * state, Hook<String> * hook);
  #endif
  bool deleteHook(const char* type, const char* name, int id);
  bool updateHook(DynamicJsonDocument doc);

  void check();
  boolean callHook(const char * type, const char * name, int id, String value);

  DynamicJsonDocument getWatchersInfo();
  DynamicJsonDocument allHooksToJson(bool ignoreReadOnly, bool shortJson);
  DynamicJsonDocument getObservableHooksJson(const char* type,
                                                 const char* name);
  DynamicJsonDocument getHookJsonById(const char* type, const char* name,
                                          int id);

  void saveHooksToSettings();

  int16_t getTotalHooksCount() { return _hooksCount; }

 private:
  #if ENABLE_SENSORS 
  List<Watcher<int16_t>> _sensorsWatchers;
  #endif

  #if ENABLE_STATES
  List<Watcher<String>> _statesWatchers;
  #endif

  int _hooksCount = 0;

  template <typename T>
  int addHook(const Observable::ObservableObject<T>* obj,
                  Hook<T>* hook);

  template <typename T>
  void collectInfo(List<Watcher<T>>* list, JsonArray* array);

  template <typename T>
  Watcher<T>* getWatcher(List<Watcher<T>>* list,
                         const Observable::ObservableObject<T>* observable);

  template <typename T>
  Watcher<T>* getWatcherByObservableName(List<Watcher<T>>* list,
                                         const char* name);

  template <typename T>
  Hook<T>* getHookFromWatcherList(List<Watcher<T>>* list,
                                          const char* name, int id);

  template <typename T>
  DynamicJsonDocument getObservableHooksJsonFromList(List<Watcher<T>>* list,
                                                         const char* name);

  template <typename T>
  DynamicJsonDocument getHookJsonFromList(List<Watcher<T>>* list,
                                              const char* name, int id);

  template <typename T>
  bool deleteHookFromList(List<Watcher<T>>* list, const char* name, int id);

  template <typename T>
  bool updateHook(List<Watcher<T>>* list, const char* name,
                      JsonObject hookObject);

  template<typename T>
  void updateTypeSpecificHookValues(Hook<T> * hook, JsonObject hookObject);

  template <typename T>
  void checkWatchers(List<Watcher<T>>* list);

  template <typename T>
  boolean callWatcherHook(List<Watcher<T>>* list, const char * name, int id, T value, boolean emptyValue);

  template <typename T>
  Watcher<T>* getWatcherOrCreate(
      const Observable::ObservableObject<T>* obj);

  template <typename T>
  Watcher<T>* createWatcher(const Observable::ObservableObject<T>* obj);

  template <typename T>
  List<Watcher<T>>* getWatchersList();
};
}  // namespace Hook

extern Hook::HooksManagerClass HooksManager;

#endif

#endif