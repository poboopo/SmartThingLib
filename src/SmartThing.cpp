#include "SmartThing.h"
#include "settings/SettingsRepository.h"

#include <ArduinoOTA.h>

#ifndef SMART_THING_LOOP_TASK_DELAY
  #define SMART_THING_LOOP_TASK_DELAY 100  // ms
#endif

#ifndef SMART_THING_HOOKS_CHECK_DELAY
  #define SMART_THING_HOOKS_CHECK_DELAY 500 // ms
#endif

#ifndef SMART_THING_BEACON_SEND_DELAY
  #define SMART_THING_BEACON_SEND_DELAY 2000 //ms
#endif

#ifndef SMART_THING_ACTIONS_SCHEDULE_DELAY
  #define SMART_THING_ACTIONS_SCHEDULE_DELAY 200 //ms
#endif

#ifndef SMART_THING_OTA_CHECK_DELAY
  #define SMART_THING_OTA_CHECK_DELAY 300 //ms
#endif

#ifndef WIPE_PIN
  #ifdef ARDUINO_ARCH_ESP32
    #define WIPE_PIN 19
  #endif
  #ifdef ARDUINO_ARCH_ESP8266
    #define WIPE_PIN D4
  #endif
#endif
#define WIPE_TIMEOUT 5000
#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP IPAddress(224, 1, 1, 1)
#define MULTICAST_PORT 7778

const char * const _SMART_THING_TAG = "smart_thing";
#ifdef ARDUINO_ARCH_ESP32
const char * const beaconTemplate = "%s;%s;%s;%s;esp32;%s";
const size_t beaconExtraSize = 10;
#endif
#ifdef ARDUINO_ARCH_ESP8266
  const char * const beaconTemplate = "%s;%s;%s;%s;esp8266;%s";
  const int beaconExtraSize = 12;

  #define WIFI_MODE_STA WIFI_STA
  #define WIFI_MODE_AP WIFI_AP
#endif
const size_t stVersionLength = strlen(SMART_THING_VERSION);

SmartThingClass SmartThing;

SmartThingClass::SmartThingClass(){};
SmartThingClass::~SmartThingClass() {
  free(_type);
  free(_ip);
  free(_broadcastMessage);
};

bool SmartThingClass::wifiConnected() {
  return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

void SmartThingClass::preInit() {
  #if ENABLE_LOGGER && LOGGER_TYPE != SERIAL_LOGGER
    ConfigManager.addConfigEntry(LOGGER_ADDRESS_CONFIG);
    #if ENABLE_TEXT_SENSORS
      SensorsManager.addSensor("logger", []() {
        return LOGGER.isConnected() ? "connected" : "disconnected";
      });
    #endif
  #endif

  #if ENABLE_CONFIG
    #if ENABLE_HOOKS
      // For notifications
      ConfigManager.addConfigEntry(GATEWAY_CONFIG);
    #endif

    ConfigManager.loadConfigValues();
    st_log_debug(_SMART_THING_TAG, "Config values loaded");
  #endif

  #if ENABLE_HOOKS
    st_log_debug(_SMART_THING_TAG, "Loading hooks from settings...");
    HooksManager.loadFromSettings();
    st_log_debug(_SMART_THING_TAG, "Hooks loaded, making first check");
    HooksManager.check();
    st_log_debug(_SMART_THING_TAG, "Hooks first check finished");
  #endif

  #if ENABLE_ACTIONS_SCHEDULER
    st_log_debug(_SMART_THING_TAG, "Loading actions schedule config from settings");
    ActionsManager.loadFromSettings();
  #endif
}

bool SmartThingClass::init(const char * type, const char * name) {
  if (name == nullptr) {
    st_log_error(_SMART_THING_TAG, "Name can't be nullptr");
    return false;
  }
  _name = (char *) malloc(strlen(name) + 1);
  strcpy(_name, name);
  return init(type);
}

bool SmartThingClass::init(const char * type) {
  if (_initialized) {
    st_log_warning(_SMART_THING_TAG, "Already initialized!");
    return false;
  }
  if (type == nullptr) {
    st_log_error(_SMART_THING_TAG, "Device type is missing!");
    return false;
  }

  preInit();

  _type = (char *) malloc(strlen(type) + 1);
  strcpy(_type, type);

  st_log_debug(_SMART_THING_TAG, "Smart thing initialization started");

  if (_name == nullptr || strlen(_name) == 0) {
    String name = SettingsRepository.getName();
    _name = (char *) malloc(name.length() + 1);
    strcpy(_name, name.c_str());
  }
  LOGGER.updateName(_name);
  st_log_debug(_SMART_THING_TAG, "Device type/name: %s/%s", _type, _name);

  st_log_debug(
    _SMART_THING_TAG,
    "Wipe pin=%d, timeout=%d",
    WIPE_PIN, WIPE_TIMEOUT
  );
  pinMode(WIPE_PIN, INPUT_PULLUP);

  st_log_debug(_SMART_THING_TAG, "Led pin=%d", LED_PIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  delay(50);
  handleWipeSettings();

  #ifdef ARDUINO_ARCH_ESP32
  st_log_debug(_SMART_THING_TAG, "Creating loop task");
  xTaskCreate([](void* o) { static_cast<SmartThingClass*>(o)->asyncLoop(); },
              _SMART_THING_TAG, 50000, this, 1, &_loopTaskHandle);
  st_log_debug(_SMART_THING_TAG, "Loop task created");
  #endif

  setupWiFi();

  st_log_info(_SMART_THING_TAG, "Setup finished");
  _initialized = true;
  return true;
}

void SmartThingClass::loop() {
  if (!_initialized) {
    return;
  }

  unsigned long current = millis();
  if (_lastBeacon == 0 || current - _lastBeacon > SMART_THING_BEACON_SEND_DELAY) {
    sendBeacon();
    _lastBeacon = current;
  }

  #if ENABLE_HOOKS
    if (_lastHooksCheck == 0 || current - _lastHooksCheck > SMART_THING_HOOKS_CHECK_DELAY) {
      HooksManager.check();
      _lastHooksCheck = current;
    }
  #endif
  
  #if ENABLE_ACTIONS_SCHEDULER
    if (_lastActionsCheck == 0 || current - _lastActionsCheck > SMART_THING_ACTIONS_SCHEDULE_DELAY) {
      ActionsManager.scheduled();
      _lastActionsCheck = current;
    }
  #endif

  #if ENABLE_OTA
    if ((_lastOtaCheck == 0 || current - _lastOtaCheck > SMART_THING_OTA_CHECK_DELAY) && wifiConnected()) {
      ArduinoOTA.handle();
      _lastOtaCheck = current;
    }
  #endif
}

#ifdef ARDUINO_ARCH_ESP32
void SmartThingClass::asyncLoop() {
  const TickType_t xDelay = SMART_THING_LOOP_TASK_DELAY / portTICK_PERIOD_MS;
  while (true) {
    loop();
    vTaskDelay(xDelay);
  }
}
#endif

void SmartThingClass::sendBeacon() {
  if (!wifiConnected() || _broadcastMessage == nullptr) {
    return;
  }
  #ifdef ARDUINO_ARCH_ESP32
    _beaconUdp.beginMulticastPacket();
  #endif
  #ifdef ARDUINO_ARCH_ESP8266
    _beaconUdp.beginPacketMulticast(MULTICAST_GROUP, MULTICAST_PORT, WiFi.localIP());
  #endif
  _beaconUdp.write((uint8_t *) _broadcastMessage, strlen(_broadcastMessage));
  _beaconUdp.endPacket();
}

void SmartThingClass::setupWiFi() {
  if (wifiConnected()) {
    st_log_info(_SMART_THING_TAG, "WiFi already connected");
    return;
  }

  _disconnectHandled = true;
  WiFi.hostname(_name);

  WiFiConfig wifiConfig = SettingsRepository.getWiFi();
  if (wifiConfig.ssid.isEmpty()) {
    st_log_warning(
      _SMART_THING_TAG,
      "Ssid is blank -> creating setup AP with name %s", ST_DEFAULT_NAME
    );
    WiFi.enableAP(true);
    WiFi.softAP(ST_DEFAULT_NAME);
    onWifiConnected();
  } else {
    st_log_info(
      _SMART_THING_TAG,
      "WiFi config: ssid=%s, password=%s, mode=%u",
      wifiConfig.ssid.c_str(),
      wifiConfig.password.c_str(),
      wifiConfig.mode
    );

    if (wifiConfig.mode == ST_WIFI_AP) {
      st_log_info(_SMART_THING_TAG, "Creating AP");
      WiFi.softAP(wifiConfig.ssid, wifiConfig.password);
      delay(200); // let it think for a sec
      onWifiConnected();
    } else if (wifiConfig.mode == ST_WIFI_STA) {
      st_log_info(_SMART_THING_TAG, "Connecting to existing AP");

      WiFi.setAutoReconnect(true);

      #ifdef ARDUINO_ARCH_ESP32
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){
          SmartThing.onWifiConnected();
        }, ARDUINO_EVENT_WIFI_STA_GOT_IP);
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){
          SmartThing.onWifiDisconnected();
        }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
      #endif

      #ifdef ARDUINO_ARCH_ESP8266
        _connectHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP event) {
          onWifiConnected();
        });
        _disconnectHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected event) {
          onWifiDisconnected();
        });
      #endif

      WiFi.enableSTA(true);
      WiFi.begin(wifiConfig.ssid, wifiConfig.password);
    } else if (wifiConfig.mode == ST_WIFI_STA_TO_AP) {
      st_log_info(_SMART_THING_TAG, "Trying to connect to existing AP");\

      WiFi.enableSTA(true);
      WiFi.begin(wifiConfig.ssid, wifiConfig.password);
      unsigned long startTime = millis();
      bool led = true;
      while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {
        digitalWrite(LED_PIN, led);
        led = !led;
        delay(50);
      }
      digitalWrite(LED_PIN, LOW);

      if (WiFi.isConnected()) {
        st_log_info(_SMART_THING_TAG, "Connected to AP");
        #ifdef ARDUINO_ARCH_ESP32
          WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){
            SmartThing.onWifiConnected();
          }, ARDUINO_EVENT_WIFI_STA_GOT_IP);
          WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info){
            SmartThing.onWifiDisconnected();
          }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        #endif

        #ifdef ARDUINO_ARCH_ESP8266
          _connectHandler = WiFi.onStationModeGotIP([this](const WiFiEventStationModeGotIP event) {
            onWifiConnected();
          });
          _disconnectHandler = WiFi.onStationModeDisconnected([this](const WiFiEventStationModeDisconnected event) {
            onWifiDisconnected();
          });
        #endif
      } else {
        WiFi.disconnect();
        st_log_error(_SMART_THING_TAG, "Failed to connect to AP, creating AP");
        WiFi.enableSTA(false);
        WiFi.enableAP(true);
        WiFi.softAP(wifiConfig.ssid, wifiConfig.password);
        delay(200);
      }
      onWifiConnected();
    } else {
      st_log_error(_SMART_THING_TAG, "WiFi mode %d not supported!", wifiConfig.mode);
    }
  }
}

void SmartThingClass::onWifiConnected() {
  st_log_debug(_SMART_THING_TAG, "WiFi connected, setting up related resources");
  _disconnectHandled = false;
  
  #if ENABLE_CONFIG
    LOGGER.connect(ConfigManager.getConfigValue(LOGGER_ADDRESS_CONFIG));
  #endif

  String ip;
  WiFiMode_t mode = WiFi.getMode();
  if (mode == WIFI_MODE_STA) {
    ip = WiFi.localIP().toString();
  } else if (mode == WIFI_MODE_AP) {
    ip = WiFi.softAPIP().toString();
  } else {
    st_log_error(_SMART_THING_TAG, "UNSUPPORTED WIFI MODE %u", mode);
  }

  if (_ip != nullptr) {
    free(_ip);
  }
  _ip = (char *) malloc(ip.length() + 1);
  strcpy(_ip, ip.c_str());

  #ifdef ARDUINO_ARCH_ESP32
    if (_beaconUdp.beginMulticast(MULTICAST_GROUP, MULTICAST_PORT)) {
      updateBroadCastMessage();
    } else {
      st_log_error(_SMART_THING_TAG, "Failed to create beacon udp");
    }
  #endif

  #ifdef ARDUINO_ARCH_ESP8266
    updateBroadCastMessage();
  #endif

  RestController.begin();
  st_log_info(_SMART_THING_TAG, "Web service started");

  #if ENABLE_OTA
    ArduinoOTA.begin();
    st_log_debug(_SMART_THING_TAG, "Ota service started");
  #endif

  st_log_info(_SMART_THING_TAG, "WiFi connected, local ip %s, hostname %s", _ip, _name);
}

void SmartThingClass::onWifiDisconnected() {
  if (_disconnectHandled) {
    return;
  }
  st_log_warning(_SMART_THING_TAG, "WiFi disconnected! Clearing resources");

  if (_broadcastMessage != nullptr) {
    free(_broadcastMessage);
    _broadcastMessage = nullptr;
  }
  if (_ip != nullptr) {
    free(_ip);
    _ip = nullptr;
  }

  _beaconUdp.stop();

  RestController.end();
  st_log_info(_SMART_THING_TAG, "Web service stopped");

  #if ENABLE_OTA
    ArduinoOTA.end();
    st_log_debug(_SMART_THING_TAG, "Ota service stoped");
  #endif

  _disconnectHandled = true;
}

void SmartThingClass::handleWipeSettings() {
  if (digitalRead(WIPE_PIN)) {
    return;
  }

  unsigned long started = millis();
  st_log_warning(_SMART_THING_TAG, "ALL SETTINGS WILL BE WIPED IN %d ms!!!",
                 WIPE_TIMEOUT);

  digitalWrite(LED_PIN, HIGH);
  while (!digitalRead(WIPE_PIN) && millis() - started < WIPE_TIMEOUT) {
    delay(50);
  }

  if (!digitalRead(WIPE_PIN)) {
    SettingsRepository.clear();
    st_log_warning(_SMART_THING_TAG, "Settings were wiped!");
  }
  digitalWrite(LED_PIN, LOW);
  ESP.restart();
}

void SmartThingClass::updateDeviceName(String name) {
  name.trim();
  name.replace(" ", "-");
  name.replace(";", "-");
  name.toLowerCase();
  if (name.equals(_name)) {
    return;
  }
  if (!SettingsRepository.setName(name)) {
    st_log_error(_SMART_THING_TAG, "Name update failed");
    return;
  }

  if (_name != nullptr && _name != ST_DEFAULT_NAME) {
    free(_name);
  }
  _name = (char *) malloc(name.length() + 1);
  strcpy(_name, name.c_str());

  updateBroadCastMessage();

  #if ENABLE_LOGGER
  LOGGER.updateName(_name);
  #endif
  
  st_log_info(_SMART_THING_TAG, "New device name %s", name.c_str());
}

void SmartThingClass::updateBroadCastMessage() {
  if (!wifiConnected()) {
    return;
  }

  if (_broadcastMessage != nullptr) {
    free(_broadcastMessage);
  }
  String version = "";
  #ifdef __VERSION
    version = String(__VERSION);
  #endif

  size_t size =
    strlen(_ip) + 
    strlen(_type) + 
    strlen(_name) + 
    stVersionLength + 
    version.length() + 
    beaconExtraSize + 1;

  _broadcastMessage = (char *) malloc(size);
  sprintf(_broadcastMessage, beaconTemplate, _ip, _type, _name, SMART_THING_VERSION, version.c_str());
}

const char * SmartThingClass::getType() { 
  return _type; 
}

const char * SmartThingClass::getName() { 
  return _name; 
}

const char * SmartThingClass::getIp() {
  return _ip;
}