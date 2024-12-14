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

  int addHook(const char * sensorName, const char * data);

  bool deleteHook(const char* name, int id);
  bool updateHook(JsonDocument doc);

  void check();
  boolean callHook(const char * name, int id, String value);

  JsonDocument getSensorHooksJson(const char* name);

  bool saveInSettings();

  int16_t getTotalHooksCount() { return _hooksCount; }

 private:
  #if ENABLE_NUMBER_SENSORS 
  List<Watcher<NUMBER_SENSOR_DATA_TYPE>> _sensorsWatchers;
  #endif

  #if ENABLE_TEXT_SENSORS
  List<Watcher<TEXT_SENSOR_DATA_TYPE>> _statesWatchers;
  #endif

  int _hooksCount = 0;

  template<typename T>
  bool loadHooks(const Sensor<T> * sensor, const char * data, int * address, int length);

  template<typename T>
  int addHook(const Sensor<T> * sensor, const char * data);

  template <typename T>
  int addHook(const Sensor<T>* obj, Hook<T>* hook);

  template <typename T>
  Watcher<T>* getWatcher(const Sensor<T>* sensor);

  template <typename T>
  Watcher<T>* getWatcherBySensorName(const char* name);

  template <typename T>
  Hook<T>* getHookFromWatcher(const char* name, int id);

  template <typename T>
  JsonDocument getSensorHooksJson(const char* name);

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