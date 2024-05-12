#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>

#include "hooks/HooksManager.h"
#include "observable/ActionsList.h"
#include "observable/ConfigEntriesList.h"
#include "observable/DeviceStatesList.h"
#include "observable/SensorsList.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/socket/Multicaster.h"
#include "settings/SettingsManager.h"
#include "utils/LedIndicator.h"
#include "Features.h"

#define SMART_THING_VERSION "0.6"
#define SMART_THING_TAG "SMART_THING"
// Pins
#define LED_PIN 13

#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

#define DEVICE_NAME_LENGTH_MAX 15
#define DEVICE_TYPE_LENGTH_MAX 15

#define SMART_THING_LOOP_TASK_DELAY 250  // ms

class SmartThingClass {
 public:
  ~SmartThingClass();
  SmartThingClass();

  bool init(String type) {
    _type = type;
    return init();
  }
  bool init(String type, String name) {
    _type = type;
    _name = name;
    return init();
  }

  void updateDeviceName(String name);
  const char * getType();
  const char * getName();
  const char * getIp();
  bool wifiConnected();

  #if ENABLE_SENSORS
  bool addSensor(
      const char* name,
      Observable::ObservableObject<int16_t>::ValueProviderFunction
          valueProvider);
  bool addDigitalSensor(const char* name, int pin);
  bool addAnalogSensor(const char* name, int pin);
  const Observable::Sensor::Sensor* getSensor(const char* name);
  DynamicJsonDocument getSensorsValues();
  int16_t getSensorsCount();
  #endif

  #if ENABLE_STATES
  bool addDeviceState(const char* name, Observable::ObservableObject<const char*>::ValueProviderFunction valueProvider);

  const Observable::DeviceState::DeviceState* getDeviceState(
      const char* name);
  DynamicJsonDocument getDeviceStatesInfo();

  int16_t getDeviceStatesCount();
  #endif

  #if ENABLE_ACTIONS
  bool addActionHandler(const char* action, const char* caption,
                        Observable::Action::ActionHandler handler);
  bool addActionHandler(const char* action,
                        Observable::Action::ActionHandler handler) {
    return addActionHandler(action, action, handler);
  };
  ActionResult callAction(const char* action);
  int16_t getActionsCount();
  DynamicJsonDocument getActionsInfo();
  #endif
  
  DynamicJsonDocument getConfigInfo();
  bool addConfigEntry(const char* name, const char* caption, const char* type);

  LedIndicator* getLed() { return &_led; }
 private:
  Multicaster _multicaster;
  LedIndicator _led;

  bool init();

  #if ENABLE_SENSORS
  Observable::Sensor::SensorsList _sensorsList;
  #endif

  #if ENABLE_STATES
  Observable::DeviceState::DeviceStatesList _deviceStatesList;
  #endif

  #if ENABLE_ACTIONS
  Observable::Action::ActionsList _actionsList;
  #endif

  Observable::Config::ConfigEntriesList _configEntriesList;

  TaskHandle_t _loopTaskHandle = NULL;
  void updateBroadCastMessage();

  String _ip;
  String _name;
  String _type;
  String _broadcastMessage;

  void wipeSettings();
  String connectToWifi();

  void loopRoutine();
};

extern SmartThingClass SmartThing;

#endif