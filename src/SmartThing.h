#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <WiFiUdp.h>

#include "hooks/HooksManager.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "config/ConfigManager.h"
#include "actions/ActionsManager.h"
#include "sensors/SensorsManager.h"
#include "Features.h"

#define SMART_THING_VERSION "1.0"
#define ST_DEFAULT_NAME "st-device"

#ifdef ARDUINO_ARCH_ESP32
#define LED_PIN 2
#endif
#ifdef ARDUINO_ARCH_ESP8266
#define LED_PIN LED_BUILTIN
#endif

static const int DEVICE_NAME_LENGTH_MAX = 16;

class SmartThingClass {
 public:
  SmartThingClass();
  ~SmartThingClass();

  /*
    Library initialization
    @param type device type
    @returns true on success
  */
  bool init(const char * type);
  /*
    Library initialization
    @param type device type
    @param name device name
    @returns true on success
  */
  bool init(const char * type, const char * name);

  #if !ENABLE_ASYNC_LOOP
    /*
      Main loop method
    */
    void loop();
  #endif

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

  #if ENABLE_OTA
    unsigned long _lastOtaCheck = 0;
  #endif

  char * _ip = nullptr;
  char * _name = nullptr;
  char * _type = nullptr;
  char * _broadcastMessage = nullptr;
  WiFiUDP _beaconUdp;

  bool _disconnectHandled = false;
  
  #if ENABLE_ASYNC_LOOP
    TaskHandle_t _loopTaskHandle = NULL;
    void loop();
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
