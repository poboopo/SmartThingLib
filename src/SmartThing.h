#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <WiFiUdp.h>

#include "hooks/HooksManager.h"
#include "settings/ConfigEntriesList.h"
#include "observable/DeviceStatesList.h"
#include "observable/SensorsList.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "settings/SettingsManager.h"
#include "actions/ActionsManager.h"
#include "Features.h"

#define SMART_THING_VERSION "0.7"

#ifdef ARDUINO_ARCH_ESP32
#define LED_PIN 2
#endif
#ifdef ARDUINO_ARCH_ESP8266
#define LED_PIN LED_BUILTIN
#define WIFI_MODE_STA WIFI_STA
#define WIFI_MODE_AP WIFI_AP
#endif

#define ST_DEFAULT_NAME "smt-device"
#define DEVICE_NAME_LENGTH_MAX 16

class SmartThingClass {
 public:
  SmartThingClass();
  ~SmartThingClass();

  bool init(const char * type);
  bool init(const char * type, const char * name);

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
  JsonDocument getSensorsTypes();
  int16_t getSensorsCount();
  #endif

  #if ENABLE_STATES
  bool addDeviceState(const char* name, Observable::ObservableObject<const char*>::ValueProviderFunction valueProvider);

  const Observable::DeviceState::DeviceState* getDeviceState(
      const char* name);
  JsonDocument getDeviceStatesInfo();

  int16_t getDeviceStatesCount();
  #endif
  
  JsonDocument getConfigInfoJson();
  Config::ConfigEntriesList * getConfigInfo() {
    return &_configEntriesList;
  }
  bool addConfigEntry(const char* name, const char* caption, const char* type);
 private:
  bool _initialized = false;
  unsigned long _lastBeacon = 0;
  #if ENABLE_HOOKS
  unsigned long _lastHooksCheck = 0;
  #endif
  #if ENABLE_ACTIONS_SCHEDULER
  unsigned long _lastActionsCheck = 0;
  #endif

  char * _ip;
  char * _name;
  char * _type;
  char * _broadcastMessage;
  WiFiUDP _beaconUdp;
  
  #ifdef ARDUINO_ARCH_ESP32
  TaskHandle_t _loopTaskHandle = NULL;
  #endif

  void updateBroadCastMessage();
  #ifdef ARDUINO_ARCH_ESP32
  void setDnsName();
  #endif

  void wipeSettings();
  void connectToWifi();

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

  Config::ConfigEntriesList _configEntriesList;
};

extern SmartThingClass SmartThing;

#endif