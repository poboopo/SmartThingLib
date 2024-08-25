#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <WiFiUdp.h>

#include "hooks/HooksManager.h"
#include "ActionsList.h"
#include "settings/ConfigEntriesList.h"
#include "observable/DeviceStatesList.h"
#include "observable/SensorsList.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "settings/SettingsManager.h"
#include "Features.h"

#define SMART_THING_VERSION "0.7"
#define SMART_THING_TAG "smart_thing"

#ifdef ARDUINO_ARCH_ESP32
#define LED_PIN 2
#endif
#ifdef ARDUINO_ARCH_ESP8266
#define LED_PIN LED_BUILTIN
#define WIFI_MODE_STA WIFI_STA
#define WIFI_MODE_AP WIFI_AP
#define LOW 1
#define HIGH 0
#endif

#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP IPAddress(224, 1, 1, 1)
#define MULTICAST_PORT 7778

#define DEVICE_NAME_LENGTH_MAX 15
#define DEVICE_TYPE_LENGTH_MAX 15

#ifndef SMART_THING_LOOP_TASK_DELAY
#define SMART_THING_LOOP_TASK_DELAY 100  // ms
#endif

#ifndef SMART_THING_HOOKS_CHECK_DELAY
#define SMART_THING_HOOKS_CHECK_DELAY 500 // ms
#endif

#ifndef SMART_THING_BEACON_SEND_DELAY
#define SMART_THING_BEACON_SEND_DELAY 1000 //ms
#endif

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

  void loop();

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
  JsonDocument getSensorsValues();
  int16_t getSensorsCount();
  #endif

  #if ENABLE_STATES
  bool addDeviceState(const char* name, Observable::ObservableObject<const char*>::ValueProviderFunction valueProvider);

  const Observable::DeviceState::DeviceState* getDeviceState(
      const char* name);
  JsonDocument getDeviceStatesInfo();

  int16_t getDeviceStatesCount();
  #endif

  #if ENABLE_ACTIONS
  bool addActionHandler(const char* action, const char* caption,
                        Action::ActionHandler handler);
  bool addActionHandler(const char* action,
                        Action::ActionHandler handler) {
    return addActionHandler(action, action, handler);
  };
  ActionResult callAction(const char* action);
  int16_t getActionsCount();
  JsonDocument getActionsInfo();
  #endif
  
  JsonDocument getConfigInfoJson();
  Config::ConfigEntriesList * getConfigInfo() {
    return &_configEntriesList;
  }
  bool addConfigEntry(const char* name, const char* caption, const char* type);
 private:
  WiFiUDP _beaconUdp;

  long _lastBeacon = -1;
  long _lastHooksCheck = -1;

  String _ip;
  String _name;
  String _type;
  String _broadcastMessage;
  
  #ifdef ARDUINO_ARCH_ESP32
  TaskHandle_t _loopTaskHandle = NULL;
  #endif

  bool init();
  void updateBroadCastMessage();

  void wipeSettings();
  String connectToWifi();

  #ifdef ARDUINO_ARCH_ESP32
  void asyncLoop();
  #endif
  void sendBeacon();

  #if ENABLE_SENSORS
  Observable::Sensor::SensorsList _sensorsList;
  #endif

  #if ENABLE_STATES
  Observable::DeviceState::DeviceStatesList _deviceStatesList;
  #endif

  #if ENABLE_ACTIONS
  Action::ActionsList _actionsList;
  #endif

  Config::ConfigEntriesList _configEntriesList;
};

extern SmartThingClass SmartThing;

#endif