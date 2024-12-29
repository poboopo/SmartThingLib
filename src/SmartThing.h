#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <WiFiUdp.h>

#include "hooks/HooksManager.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "settings/SettingsRepository.h"
#include "actions/ActionsManager.h"
#include "sensors/SensorsManager.h"
#include "Features.h"

#define SMART_THING_VERSION "0.7"

#ifdef ARDUINO_ARCH_ESP32
#define LED_PIN 2
#endif
#ifdef ARDUINO_ARCH_ESP8266
#define LED_PIN LED_BUILTIN
#endif

static const char * ST_DEFAULT_NAME = "st-device";
static const int DEVICE_NAME_LENGTH_MAX = 16;

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
 private:
  bool _initialized = false;
  unsigned long _lastBeacon = 0;

  #if ENABLE_HOOKS
    unsigned long _lastHooksCheck = 0;
  #endif

  #if ENABLE_ACTIONS_SCHEDULER
    unsigned long _lastActionsCheck = 0;
  #endif

  char * _ip = nullptr;
  char * _name = nullptr;
  char * _type = nullptr;
  char * _broadcastMessage = nullptr;
  WiFiUDP _beaconUdp;

  bool _disconnectHandled = false;
  
  #ifdef ARDUINO_ARCH_ESP32
    TaskHandle_t _loopTaskHandle = NULL;
    void setDnsName();
    void asyncLoop();
  #endif

  #ifdef ARDUINO_ARCH_ESP8266
    WiFiEventHandler _connectHandler;
    WiFiEventHandler _disconnectHandler;
  #endif

  void preInit();
  void handleWipeSettings();

  void setupWiFi();
  void onWifiConnected();
  void onWifiDisconnected();

  void updateBroadCastMessage();
  void sendBeacon();
};

extern SmartThingClass SmartThing;

#endif