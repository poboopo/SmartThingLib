#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include <ArduinoJson.h>

#include <functional>

#include "hooks/impls/Hook.h"
#include "hooks/impls/LambdaHook.h"
#include "hooks/watchers/DeviceStateWatcher.h"
#include "hooks/watchers/SensorWatcher.h"
#include "hooks/watchers/Watcher.h"
#include "configurable/ConfigurableObjects.h"
#include "utils/List.h"

namespace Hook {
class HooksManagerClass {
 public:
  void loadFromSettings();
  int createHookFromJson(const char* json);
  int createHookFromJson(JsonObject observableInfo,
                             JsonObject hookInfo);

  int addHook(const Configurable::Sensor::Sensor * sensor, Hook<int16_t> * hook);
  int addHook(const Configurable::DeviceState::DeviceState * state, Hook<String> * hook);
  bool deleteHook(const char* type, const char* name, int id);
  bool updateHook(DynamicJsonDocument doc);

  void check();

  DynamicJsonDocument getWatchersInfo();
  DynamicJsonDocument allHooksToJson(bool ignoreReadOnly, bool shortJson);
  DynamicJsonDocument getObservableHooksJson(const char* type,
                                                 const char* name);
  DynamicJsonDocument getHookJsonById(const char* type, const char* name,
                                          int id);

  void saveHooksToSettings();

  int16_t getTotalHooksCount() { return _hooksCount; }

 private:
  List<Watcher<int16_t>> _sensorsWatchers;
  List<Watcher<String>> _statesWatchers;
  int _hooksCount = 0;

  template <typename T>
  int addHook(const Configurable::ConfigurableObject<T>* obj,
                  Hook<T>* hook);

  template <typename T>
  void collectInfo(List<Watcher<T>>* list, JsonArray* array);

  template <typename T>
  Watcher<T>* getWatcher(List<Watcher<T>>* list,
                         const Configurable::ConfigurableObject<T>* observable);

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
  Watcher<T>* getWatcherOrCreate(
      const Configurable::ConfigurableObject<T>* obj);

  template <typename T>
  Watcher<T>* createWatcher(const Configurable::ConfigurableObject<T>* obj);

  template <typename T>
  List<Watcher<T>>* getWatchersList();
};
}  // namespace Hook

extern Hook::HooksManagerClass HooksManager;

#endif