#include "SmartThing.h"

#define WIPE_PIN 19
#define WIPE_TIMEOUT 5000
#define WIFI_SETUP_TIMEOUT 10000

#define MULTICAST_GROUP IPAddress(224, 1, 1, 1)
#define MULTICAST_PORT 7778

SmartThingClass SmartThing;

SmartThingClass::SmartThingClass(){};
SmartThingClass::~SmartThingClass(){};

bool SmartThingClass::wifiConnected() {
  return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

bool SmartThingClass::init() {
  if (_initialized) {
    LOGGER.warning(SMART_THING_TAG, "Already initialized!");
    return false;
  }
  LOGGER.debug(SMART_THING_TAG, "Smart thing initialization started");

  STSettings.loadSettings();
  LOGGER.debug(SMART_THING_TAG, "Settings manager loaded");

  _name = STSettings.getDeviceName();
  LOGGER.debug(SMART_THING_TAG, "Device type/name: %s/%s", _type.c_str(), _name.c_str());

  LOGGER.debug(
    SMART_THING_TAG,
    "Wipe pin=%d, timeout=%d",
    WIPE_PIN, WIPE_TIMEOUT
  );
  pinMode(WIPE_PIN, INPUT_PULLUP);
  LOGGER.debug(SMART_THING_TAG, "Led pin=%d", LED_PIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  delay(50);
  // todo esp8266
  if (!digitalRead(WIPE_PIN)) {
    wipeSettings();
  }

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

  _ip = connectToWifi();

  if (wifiConnected()) {
    LOGGER.info(SMART_THING_TAG, "WiFi connected, local ip %s, hostname %s", _ip.c_str(), _name.c_str());
    delay(1000);
    LOGGER.init(STSettings.getConfig()[LOGGER_ADDRESS_CONFIG], _name.c_str());

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

    RestController.begin();
    LOGGER.info(SMART_THING_TAG, "RestController started");
  } else {
    LOGGER.warning(SMART_THING_TAG,
                   "WiFi not available, skipping all network setup");
  }

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
  if (!wifiConnected()) {
    return;
  }
  #ifdef ARDUINO_ARCH_ESP32
  _beaconUdp.beginMulticastPacket();
  #endif
  #ifdef ARDUINO_ARCH_ESP8266
  _beaconUdp.beginPacketMulticast(MULTICAST_GROUP, MULTICAST_PORT, WiFi.localIP());
  #endif
  _beaconUdp.write((uint8_t *) _broadcastMessage.c_str(), _broadcastMessage.length());
  _beaconUdp.endPacket();
}

String SmartThingClass::connectToWifi() {
  LOGGER.info(SMART_THING_TAG, "Trying to connect to wifi");
  if (wifiConnected()) {
    LOGGER.info(SMART_THING_TAG, "WiFi already connected");
    return WiFi.localIP().toString();
  }
  WiFi.setHostname(_name.c_str());
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
    return WiFi.softAPIP().toString();
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
      return WiFi.softAPIP().toString();
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
        return WiFi.localIP().toString();
      } else {
        WiFi.disconnect();
        LOGGER.error(SMART_THING_TAG, "Failed to connect to Wifi (%s::%s)", ssid, password);
        return "";
      }
    } else {
      LOGGER.error(SMART_THING_TAG, "Mode %d not supported!", mode);
      return "";
    }
  }
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
  name.replace(" ", "_");
  if (name == _name) {
    return;
  }
  _name = name;
  STSettings.setDeviceName(_name.c_str());
  STSettings.save();
  updateBroadCastMessage();
  LOGGER.info(SMART_THING_TAG, "New device name %s", name.c_str());
}

void SmartThingClass::updateBroadCastMessage() {
  _broadcastMessage = _ip + "$" + _type + "$" + _name + "$" + SMART_THING_VERSION;
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
  return _type.c_str(); 
}

const char * SmartThingClass::getName() { 
  return _name.c_str(); 
}

const char * SmartThingClass::getIp() {
  return _ip.c_str();
}