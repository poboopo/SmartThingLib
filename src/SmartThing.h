#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>

#include "hooks/HooksManager.h"
#include "configurable/ActionsList.h"
#include "configurable/ConfigEntriesList.h"
#include "configurable/DeviceStatesList.h"
#include "configurable/SensorsList.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/socket/Multicaster.h"
#include "settings/SettingsManager.h"
#include "utils/LedIndicator.h"

#define SMART_THING_VERSION "0.5"
#define SMART_THING_TAG "SMART_THING"
// Pins
#define LED_PIN 13

#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

#define WEB_PAGE

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

  bool addSensor(
      const char* name,
      Configurable::ConfigurableObject<int16_t>::ValueProviderFunction
          valueProvider);
  bool addDigitalSensor(const char* name, int pin);
  bool addAnalogSensor(const char* name, int pin);
  bool addDeviceState(
      const char* name,
      Configurable::ConfigurableObject<const char*>::ValueProviderFunction
          valueProvider);
  bool addActionHandler(const char* action, const char* caption,
                        Configurable::Action::ActionHandler handler);
  bool addActionHandler(const char* action,
                        Configurable::Action::ActionHandler handler) {
    return addActionHandler(action, action, handler);
  };
  ActionResult callAction(const char* action);
  bool addConfigEntry(const char* name, const char* caption, const char* type);

  const Configurable::DeviceState::DeviceState* getDeviceState(
      const char* name);
  const Configurable::Sensor::Sensor* getSensor(const char* name);

  DynamicJsonDocument getSensorsValues();
  DynamicJsonDocument getDeviceStatesInfo();
  DynamicJsonDocument getActionsInfo();
  DynamicJsonDocument getConfigInfo();
  DynamicJsonDocument getWatchersInfo();

  int16_t getActionsCount() { return _actionsList.size(); }

  int16_t getSensorsCount() { return _sensorsList.size(); }

  int16_t getDeviceStatesCount() { return _deviceStatesList.size(); }

  LedIndicator* getLed() { return &_led; }

 private:
  Multicaster _multicaster;
  LedIndicator _led;

  bool init();

  Configurable::Sensor::SensorsList _sensorsList;
  Configurable::DeviceState::DeviceStatesList _deviceStatesList;
  Configurable::Action::ActionsList _actionsList;
  Configurable::Config::ConfigEntriesList _configEntriesList;

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