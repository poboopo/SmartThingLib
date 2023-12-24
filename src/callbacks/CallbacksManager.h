#ifndef WATCHERS_LIST_H
#define WATCHERS_LIST_H

#include <ArduinoJson.h>

#include <functional>

#include "callbacks/impls/Callback.h"
#include "callbacks/impls/LambdaCallback.h"
#include "callbacks/watchers/DeviceStateWatcher.h"
#include "callbacks/watchers/SensorWatcher.h"
#include "callbacks/watchers/Watcher.h"
#include "configurable/ConfigurableObjects.h"
#include "utils/List.h"

namespace Callback {
class CallbacksManagerClass {
 public:
  void loadFromSettings();
  int createCallbackFromJson(const char* json);
  int createCallbackFromJson(JsonObject observableInfo,
                             JsonObject callbackInfo);

  template <typename T>
  int addCallback(const Configurable::ConfigurableObject<T>* obj,
                  Callback<T>* callback);
  bool deleteCallback(const char* type, const char* name, int id);
  bool updateCallback(DynamicJsonDocument doc);

  void check();

  DynamicJsonDocument getWatchersInfo();
  DynamicJsonDocument allCallbacksToJson(bool ignoreReadOnly, bool shortJson);
  DynamicJsonDocument getObservableCallbacksJson(const char* type,
                                                 const char* name);
  DynamicJsonDocument getCallbackJsonById(const char* type, const char* name,
                                          int id);

  void saveCallbacksToSettings();

  int16_t getTotalCallbacksCount() { return _callbacksCount; }

 private:
  List<Watcher<int16_t>> _sensorsWatchers;
  List<Watcher<String>> _statesWatchers;
  int _callbacksCount = 0;

  template <typename T>
  void collectInfo(List<Watcher<T>>* list, JsonArray* array);

  template <typename T>
  Watcher<T>* getWatcher(List<Watcher<T>>* list,
                         const Configurable::ConfigurableObject<T>* observable);

  template <typename T>
  Watcher<T>* getWatcherByObservableName(List<Watcher<T>>* list,
                                         const char* name);

  template <typename T>
  Callback<T>* getCallbackFromWatcherList(List<Watcher<T>>* list,
                                          const char* name, int id);

  template <typename T>
  DynamicJsonDocument getObservableCallbacksJsonFromList(List<Watcher<T>>* list,
                                                         const char* name);

  template <typename T>
  DynamicJsonDocument getCallbackJsonFromList(List<Watcher<T>>* list,
                                              const char* name, int id);

  template <typename T>
  bool deleteCallbackFromList(List<Watcher<T>>* list, const char* name, int id);

  template <typename T>
  bool updateCallback(List<Watcher<T>>* list, const char* name,
                      JsonObject callbackObject);

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
}  // namespace Callback

extern Callback::CallbacksManagerClass CallbacksManager;

#endif