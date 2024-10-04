#include "SmartThing.h"
#include <mdns.h>

#define WIPE_PIN 19
#define WIPE_TIMEOUT 5000
#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP IPAddress(224, 1, 1, 1)
#define MULTICAST_PORT 7778

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

bool SmartThingClass::init(const char * type) {
  if (_initialized) {
    LOGGER.warning(SMART_THING_TAG, "Already initialized!");
    return false;
  }
  if (type == nullptr) {
    LOGGER.error(SMART_THING_TAG, "Device type is missing!");
    return false;
  }

  _type = (char *) malloc(strlen(type) + 1);
  strcpy(_type, type);

  LOGGER.debug(SMART_THING_TAG, "Smart thing initialization started");

  STSettings.loadSettings();
  LOGGER.debug(SMART_THING_TAG, "Settings manager loaded");

  if (_name == nullptr || strlen(_name) == 0) {
    String name = STSettings.getDeviceName();
    _name = (char *) malloc(name.length() + 1);
    strcpy(_name, name.c_str());
  }
  LOGGER.debug(SMART_THING_TAG, "Device type/name: %s/%s", _type, _name);

  #ifdef ARDUINO_ARCH_ESP32
  LOGGER.debug(
    SMART_THING_TAG,
    "Wipe pin=%d, timeout=%d",
    WIPE_PIN, WIPE_TIMEOUT
  );
  pinMode(WIPE_PIN, INPUT_PULLUP);
  #endif
  LOGGER.debug(SMART_THING_TAG, "Led pin=%d", LED_PIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  delay(50);
  // todo esp8266
  #ifdef ARDUINO_ARCH_ESP32
  if (!digitalRead(WIPE_PIN)) {
    wipeSettings();
  }
  #endif

  #ifdef ARDUINO_ARCH_ESP32
  LOGGER.debug(SMART_THING_TAG, "Creating loop task");
  xTaskCreate([](void* o) { static_cast<SmartThingClass*>(o)->asyncLoop(); },
              SMART_THING_TAG, 50000, this, 1, &_loopTaskHandle);
  LOGGER.debug(SMART_THING_TAG, "Loop task created");
  #endif

  #if ENABLE_LOGGER && LOGGER_TYPE != SERIAL_LOGGER
  addConfigEntry(LOGGER_ADDRESS_CONFIG, "Logger address (ip:port)", "string");
    #if ENABLE_STATES
  addDeviceState("logger", []() {
    return LOGGER.isConnected() ? "connected" : "disconnected";
  });
    #endif
  #endif
  #if ENABLE_HOOKS
  // For notifications
  addConfigEntry(GATEWAY_CONFIG, "Gateway address (ip:port)", "string");
  #endif

  #if ENABLE_STATES
  addDeviceState("wifi", [this]() {
    return wifiConnected() ? "connected" : "disconnected";
  });
  #endif

  #if ENABLE_HOOKS
  LOGGER.debug(SMART_THING_TAG, "Loading hooks from settings...");
  HooksManager.loadFromSettings();
  LOGGER.debug(SMART_THING_TAG, "Hooks loaded, making first check");
  HooksManager.check();
  LOGGER.debug(SMART_THING_TAG, "Hooks first check finished");
  #endif

  connectToWifi();

  if (wifiConnected()) {
    LOGGER.info(SMART_THING_TAG, "WiFi connected, local ip %s, hostname %s", _ip, _name);
    delay(1000);
    LOGGER.init(STSettings.getConfig()[LOGGER_ADDRESS_CONFIG], _name);

    #ifdef ARDUINO_ARCH_ESP32
    if (_beaconUdp.beginMulticast(MULTICAST_GROUP, MULTICAST_PORT)) {
      updateBroadCastMessage();
      LOGGER.info(SMART_THING_TAG, "Beacon udp created");
    } else {
      LOGGER.error(SMART_THING_TAG, "Failed to create beacon udp");
    }
    #endif
    #ifdef ARDUINO_ARCH_ESP8266
    updateBroadCastMessage();
    #endif

    esp_err_t errInit = mdns_init();
    if (errInit != ESP_OK) {
      LOGGER.error(SMART_THING_TAG, "Failed to init mdns! (code=%s)", esp_err_to_name(errInit));
    } else {
      LOGGER.info(SMART_THING_TAG, "Mnds initialized");
      // todo smh mdns not working after first hostname sets
      mdns_hostname_set("");
      setDnsName();
    }

    RestController.begin();
    LOGGER.info(SMART_THING_TAG, "RestController started");
  } else {
    LOGGER.warning(SMART_THING_TAG,
                   "WiFi not available, skipping all network setup");
  }

  LOGGER.debug(SMART_THING_TAG, "Setup finished");
  _initialized = true;
  return true;
}

void SmartThingClass::loop() {
  if (_lastBeacon == -1 || millis() - _lastBeacon > SMART_THING_BEACON_SEND_DELAY) {
    sendBeacon();
    _lastBeacon = millis();
  }
  #if ENABLE_HOOKS
  if (_lastHooksCheck == -1 || millis() - _lastHooksCheck > SMART_THING_HOOKS_CHECK_DELAY) {
    HooksManager.check();
    _lastHooksCheck = millis();
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

// todo move to different async task
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
  LOGGER.info(SMART_THING_TAG, "Trying to connect to wifi");
  String ip = "";
  if (wifiConnected()) {
    LOGGER.info(SMART_THING_TAG, "WiFi already connected");
    return;
  }
  
  WiFi.hostname(_name);
  WiFi.setAutoReconnect(true);

  JsonObject wifiConfig = STSettings.getWiFi();
  const char* ssid = wifiConfig[SSID_SETTING];
  const char* password = wifiConfig[PASSWORD_SETTING];
  int mode = wifiConfig[WIFI_MODE_SETTING];

  if (ssid == nullptr || strlen(ssid) == 0) {
    LOGGER.warning(
      SMART_THING_TAG,
      "Ssid is blank or mode null -> creating setup AP with name %s", SMT_DEFAULT_NAME
    );
    WiFi.softAP(SMT_DEFAULT_NAME);
    delay(500);
    LOGGER.info(SMART_THING_TAG, "WiFi started in soft AP mode");
    ip = WiFi.softAPIP().toString();
  } else {
    if (mode == WIFI_MODE_AP) {
      if (password != nullptr && strlen(password) >= 0) {
        LOGGER.info(SMART_THING_TAG, "Creating AP point %s :: %s", ssid,
                    password);
        WiFi.softAP(ssid, password);
      } else {
        LOGGER.info(SMART_THING_TAG, "Creating AP point %s", ssid);
        WiFi.softAP(ssid);
      }
      delay(500);
      LOGGER.info(SMART_THING_TAG, "WiFi started in AP mode");
      ip = WiFi.softAPIP().toString();
    } else if (mode == WIFI_MODE_STA) {
      LOGGER.debug(SMART_THING_TAG, "WiFi connecting to %s :: %s", ssid,
                   password);
      WiFi.begin(ssid, password);
      long startTime = millis();
      bool led = true;
      while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {
        digitalWrite(LED_PIN, led);
        led = !led;
        delay(50);
      }
      digitalWrite(LED_PIN, LOW);
      if (WiFi.isConnected()) {
        LOGGER.info(SMART_THING_TAG, "WiFi started in STA mode"); 
        ip = WiFi.localIP().toString();
      } else {
        WiFi.disconnect();
        LOGGER.error(SMART_THING_TAG, "Failed to connect to Wifi (%s::%s)", ssid, password);
      }
    } else {
      LOGGER.error(SMART_THING_TAG, "Mode %d not supported!", mode);
    }
  }
  _ip = (char *) malloc(ip.length() + 1);
  strcpy(_ip, ip.c_str());
}

void SmartThingClass::wipeSettings() {
  long started = millis();
  LOGGER.warning(SMART_THING_TAG, "ALL SETTINGS WILL BE WIPED IN %d ms!!!",
                 WIPE_TIMEOUT);

  digitalWrite(LED_PIN, HIGH);
  while (!digitalRead(WIPE_PIN) &&
         millis() - started < WIPE_TIMEOUT) {
  }
  if (!digitalRead(WIPE_PIN)) {
    STSettings.wipeAll();
    STSettings.save();
    LOGGER.warning(SMART_THING_TAG, "Settings were wiped!");
  }
  digitalWrite(LED_PIN, LOW);
}

void SmartThingClass::updateDeviceName(String name) {
  name.trim();
  name.replace(" ", "-");
  name.toLowerCase();
  if (name.equals(_name)) {
    return;
  }
  free(_name);
  _name = (char *) malloc(name.length() + 1);
  strcpy(_name, name.c_str());

  STSettings.setDeviceName(_name);
  STSettings.save();

  updateBroadCastMessage();
  setDnsName();
  
  LOGGER.info(SMART_THING_TAG, "New device name %s", name.c_str());
}

void SmartThingClass::updateBroadCastMessage() {
  free(_broadcastMessage);
  _broadcastMessage = (char *) malloc(strlen(_ip) + strlen(_type) + strlen(_name) + strlen(SMART_THING_VERSION) + 4);
  sprintf(_broadcastMessage, "%s$%s$%s$%s", _ip, _type, _name, SMART_THING_VERSION);
}

void SmartThingClass::setDnsName() {
  // todo add hostname exists check
  char * hostname = (char *) malloc(strlen(_name) + 5);
  sprintf(hostname, "%s-smt", _name);
  esp_err_t err = mdns_hostname_set(hostname);
  if (err != ESP_OK) {
    LOGGER.error(SMART_THING_TAG, "Failed to set mdns hostname! (code=%s)", esp_err_to_name(err));
  } else {
    LOGGER.info(SMART_THING_TAG, "New mdns hostname: %s", hostname);
  }
  free(hostname);
}

JsonDocument SmartThingClass::getConfigInfoJson() {
  return _configEntriesList.toJson();
}

#if ENABLE_SENSORS 
JsonDocument SmartThingClass::getSensorsValues() {
  return _sensorsList.getValues();
} 
JsonDocument SmartThingClass::getSensorsTypes() {
  return _sensorsList.getTypes();
}

int16_t SmartThingClass::getSensorsCount() { return _sensorsList.size(); }

bool SmartThingClass::addSensor(
    const char* name,
    Observable::ObservableObject<int16_t>::ValueProviderFunction function) {
  return _sensorsList.add(name, function);
}

bool SmartThingClass::addDigitalSensor(const char* name, int pin) {
  pinMode(pin, INPUT_PULLUP);
  return _sensorsList.addDigital(name, pin);
}

bool SmartThingClass::addAnalogSensor(const char* name, int pin) {
  return _sensorsList.addAnalog(name, pin);
}

const Observable::Sensor::Sensor* SmartThingClass::getSensor(
    const char* name) {
  return _sensorsList.findSensor(name);
}
#endif

#if ENABLE_STATES
bool SmartThingClass::addDeviceState(
    const char* name,
    Observable::ObservableObject<const char*>::ValueProviderFunction
        function) {
  return _deviceStatesList.add(name, function);
}

JsonDocument SmartThingClass::getDeviceStatesInfo() {
  return _deviceStatesList.getValues();
}

const Observable::DeviceState::DeviceState* SmartThingClass::getDeviceState(
    const char* name) {
  return _deviceStatesList.findState(name);
}

int16_t SmartThingClass::getDeviceStatesCount() {
  return _deviceStatesList.size();
}

#endif

#if ENABLE_ACTIONS 
JsonDocument SmartThingClass::getActionsInfo() {
  return _actionsList.toJson();
}
bool SmartThingClass::addActionHandler(const char* action, const char* caption,
                                       Action::ActionHandler handler) {
  return _actionsList.add(action, caption, handler);
}

ActionResult SmartThingClass::callAction(const char* action) {
  return _actionsList.callAction(action);
}

int16_t SmartThingClass::getActionsCount() {
  return _actionsList.size();
}
#endif

bool SmartThingClass::addConfigEntry(const char* name, const char* caption,
                                     const char* type) {
  return _configEntriesList.add(name, caption, type);
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