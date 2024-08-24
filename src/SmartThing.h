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
#include "utils/LedIndicator.h"
#include "Features.h"

#ifdef ARDUINO_ARCH_ESP8266
#define WIFI_MODE_STA WIFI_STA
#define WIFI_MODE_AP WIFI_AP
#endif

#define SMART_THING_VERSION "0.7"
#define SMART_THING_TAG "SMART_THING"
// Pins
#define LED_PIN 13

#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP IPAddress(224, 1, 1, 1)
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


  #ifdef ARDUINO_ARCH_ESP8266
  void loop() {
    loopRoutine();
  }
  #endif

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
                        Action::ActionHandler handler);
  bool addActionHandler(const char* action,
                        Action::ActionHandler handler) {
    return addActionHandler(action, action, handler);
  };
  ActionResult callAction(const char* action);
  int16_t getActionsCount();
  DynamicJsonDocument getActionsInfo();
  #endif
  
  DynamicJsonDocument getConfigInfoJson();
  Config::ConfigEntriesList * getConfigInfo() {
    return &_configEntriesList;
  }
  bool addConfigEntry(const char* name, const char* caption, const char* type);

  LedIndicator* getLed() { return &_led; }
 private:
  LedIndicator _led;
  WiFiUDP _beaconUdp;

  bool init();
  
  #ifdef ARDUINO_ARCH_ESP32
  TaskHandle_t _loopTaskHandle = NULL;
  #endif
  void updateBroadCastMessage();

  String _ip;
  String _name;
  String _type;
  String _broadcastMessage;

  void wipeSettings();
  String connectToWifi();

  void loopRoutine();
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