#include "SmartThing.h"

#ifdef ARDUINO_ARCH_ESP32
#include <mdns.h>
#endif

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
const size_t beaconExtraSize = 9;
#endif
#ifdef ARDUINO_ARCH_ESP8266
const char * const beaconTemplate = "%s;%s;%s;%s;esp8266;%s";
const int beaconExtraSize = 11;
#endif
const size_t versionLen = strlen(SMART_THING_VERSION);

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

  _type = (char *) malloc(strlen(type) + 1);
  strcpy(_type, type);

  st_log_debug(_SMART_THING_TAG, "Smart thing initialization started");

  if (_name == nullptr || strlen(_name) == 0) {
    String name = SettingsRepository.getName();
    _name = (char *) malloc(name.length() + 1);
    strcpy(_name, name.c_str());
  }
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
  if (!digitalRead(WIPE_PIN)) {
    wipeSettings();
  }

  #ifdef ARDUINO_ARCH_ESP32
  st_log_debug(_SMART_THING_TAG, "Creating loop task");
  xTaskCreate([](void* o) { static_cast<SmartThingClass*>(o)->asyncLoop(); },
              _SMART_THING_TAG, 50000, this, 1, &_loopTaskHandle);
  st_log_debug(_SMART_THING_TAG, "Loop task created");
  #endif

  #if ENABLE_LOGGER && LOGGER_TYPE != SERIAL_LOGGER
  SettingsRepository.addConfigEntry(LOGGER_ADDRESS_CONFIG, "Logger address (ip:port)");
    #if ENABLE_STATES
    ObservablesManager.addDeviceState("logger", []() {
      return LOGGER.isConnected() ? "connected" : "disconnected";
    });
    #endif
  #endif

  #if ENABLE_STATES
  ObservablesManager.addDeviceState("wifi", [this]() {
    return wifiConnected() ? "connected" : "disconnected";
  });
  #endif

  #if ENABLE_HOOKS
  // For notifications
  SettingsRepository.addConfigEntry(GATEWAY_CONFIG, "Gateway address (ip:port)");

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

  connectToWifi();

  if (wifiConnected()) {
    st_log_info(_SMART_THING_TAG, "WiFi connected, local ip %s, hostname %s", _ip, _name);
    delay(1000);
    LOGGER.init(SettingsRepository.getConfig()[LOGGER_ADDRESS_CONFIG], _name);

    #ifdef ARDUINO_ARCH_ESP32
    if (_beaconUdp.beginMulticast(MULTICAST_GROUP, MULTICAST_PORT)) {
      updateBroadCastMessage();
      st_log_info(_SMART_THING_TAG, "Beacon udp created");
    } else {
      st_log_error(_SMART_THING_TAG, "Failed to create beacon udp");
    }
    #endif
    #ifdef ARDUINO_ARCH_ESP8266
    updateBroadCastMessage();
    #endif
    
    #ifdef ARDUINO_ARCH_ESP32
    esp_err_t errInit = mdns_init();
    if (errInit != ESP_OK) {
      st_log_error(_SMART_THING_TAG, "Failed to init mdns! (code=%s)", esp_err_to_name(errInit));
    } else {
      st_log_info(_SMART_THING_TAG, "Mnds initialized");
      // todo smh mdns not working after first hostname sets
      mdns_hostname_set("");
      setDnsName();
    }
    #endif

    RestController.begin();
    st_log_info(_SMART_THING_TAG, "RestController started");
  } else {
    st_log_warning(_SMART_THING_TAG,
                   "WiFi not available, skipping all network setup");
  }

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

void SmartThingClass::connectToWifi() {
  st_log_info(_SMART_THING_TAG, "Trying to connect to wifi");
  String ip = "";
  if (wifiConnected()) {
    st_log_info(_SMART_THING_TAG, "WiFi already connected");
    return;
  }
  
  WiFi.hostname(_name);
  WiFi.setAutoReconnect(true);

  WiFiConfig wifiConfig = SettingsRepository.getWiFi();
  if (wifiConfig.ssid.isEmpty()) {
    st_log_warning(
      _SMART_THING_TAG,
      "Ssid is blank or mode null -> creating setup AP with name %s", ST_DEFAULT_NAME
    );
    WiFi.softAP(ST_DEFAULT_NAME);
    delay(500);
    st_log_info(_SMART_THING_TAG, "WiFi started in soft AP mode");
    ip = WiFi.softAPIP().toString();
  } else {
    if (wifiConfig.mode == WIFI_MODE_AP) {
      if (!wifiConfig.password.isEmpty()) {
        st_log_info(
          _SMART_THING_TAG,
          "Creating AP point %s :: %s", 
          wifiConfig.ssid.c_str(),
          wifiConfig.password.c_str()
        );
        WiFi.softAP(wifiConfig.ssid, wifiConfig.password);
      } else {
        st_log_info(_SMART_THING_TAG, "Creating AP point %s", wifiConfig.ssid.c_str());
        WiFi.softAP(wifiConfig.ssid);
      }
      delay(500);
      st_log_info(_SMART_THING_TAG, "WiFi started in AP mode");
      ip = WiFi.softAPIP().toString();
    } else if (wifiConfig.mode == WIFI_MODE_STA) {
      st_log_debug(
        _SMART_THING_TAG,
        "WiFi connecting to %s :: %s",
        wifiConfig.ssid.c_str(),
        wifiConfig.password.c_str()
      );
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
        st_log_info(_SMART_THING_TAG, "WiFi started in STA mode"); 
        ip = WiFi.localIP().toString();
      } else {
        WiFi.disconnect();
        st_log_error(
          _SMART_THING_TAG,
          "Failed to connect to Wifi (%s::%s)", 
          wifiConfig.ssid.c_str(),
          wifiConfig.password.c_str()
        );
      }
    } else {
      st_log_error(_SMART_THING_TAG, "Mode %d not supported!", wifiConfig.mode);
    }
  }
  _ip = (char *) malloc(ip.length() + 1);
  strcpy(_ip, ip.c_str());
}

void SmartThingClass::wipeSettings() {
  unsigned long started = millis();
  st_log_warning(_SMART_THING_TAG, "ALL SETTINGS WILL BE WIPED IN %d ms!!!",
                 WIPE_TIMEOUT);

  digitalWrite(LED_PIN, HIGH);
  while (!digitalRead(WIPE_PIN) &&
         millis() - started < WIPE_TIMEOUT) {
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
  free(_name);
  _name = (char *) malloc(name.length() + 1);
  strcpy(_name, name.c_str());

  updateBroadCastMessage();

  #if ENABLE_LOGGER
  LOGGER.updateName(_name);
  #endif

  #ifdef ARDUINO_ARCH_ESP32
  setDnsName();
  #endif
  
  st_log_info(_SMART_THING_TAG, "New device name %s", name.c_str());
}

void SmartThingClass::updateBroadCastMessage() {
  if (_broadcastMessage != nullptr) {
    free(_broadcastMessage);
  }
  size_t size = strlen(_ip) + strlen(_type) + strlen(_name) + versionLen + beaconExtraSize + 1;
  _broadcastMessage = (char *) malloc(size);
  #ifdef __VERSION
    sprintf(_broadcastMessage, beaconTemplate, _ip, _type, _name, SMART_THING_VERSION, String(__VERSION).c_str());
  #else
    sprintf(_broadcastMessage, beaconTemplate, _ip, _type, _name, SMART_THING_VERSION, "");
  #endif
}

#ifdef ARDUINO_ARCH_ESP32
void SmartThingClass::setDnsName() {
  // todo add hostname exists check
  char * hostname = (char *) malloc(strlen(_name) + 5);
  sprintf(hostname, "%s-st", _name);
  esp_err_t err = mdns_hostname_set(hostname);
  if (err != ESP_OK) {
    st_log_error(_SMART_THING_TAG, "Failed to set mdns hostname! (code=%s)", esp_err_to_name(err));
  } else {
    st_log_info(_SMART_THING_TAG, "New mdns hostname: %s", hostname);
  }
  free(hostname);
}
#endif

const char * SmartThingClass::getType() { 
  return _type; 
}

const char * SmartThingClass::getName() { 
  return _name; 
}

const char * SmartThingClass::getIp() {
  return _ip;
}