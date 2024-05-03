#include "SmartThing.h"

#define WIPE_PIN 19
#define WIPE_TIMEOUT 5000

using namespace Configurable;

SmartThingClass SmartThing;

SmartThingClass::SmartThingClass(){};
SmartThingClass::~SmartThingClass(){};

bool SmartThingClass::wifiConnected() {
  return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

bool SmartThingClass::init() {
  LOGGER.init();
  LOGGER.debug(SMART_THING_TAG, "Smart thing initialization started");

  STSettings.loadSettings();
  LOGGER.debug(SMART_THING_TAG, "Settings manager loaded");

  _name = STSettings.getDeviceName();
  if (_name.isEmpty()) {
    _name = ESP.getChipModel();
  }
  LOGGER.debug(SMART_THING_TAG, "Device type/name: %s/%s", _type, _name);

  LOGGER.debug(
    SMART_THING_TAG,
    "Wipe pin=%d, timeout=%d",
    WIPE_PIN, WIPE_TIMEOUT
  );
  pinMode(WIPE_PIN, INPUT_PULLUP);
  _led.init(LED_PIN);

  delay(50);
  if (!digitalRead(WIPE_PIN)) {
    wipeSettings();
  }

  _ip = connectToWifi();

  if (wifiConnected()) {
    LOGGER.info(SMART_THING_TAG, "WiFi connected, local ip %s", _ip);
    delay(1000);
    LOGGER.initNetConnection(STSettings.getConfig()[LOGGER_ADDRESS_CONFIG], _name.c_str());

    LOGGER.debug(SMART_THING_TAG, "Ota started");

    _multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
    updateBroadCastMessage();
    LOGGER.debug(SMART_THING_TAG, "Multicaster created");

    RestController.begin();
    LOGGER.debug(SMART_THING_TAG, "RestController started");
  } else {
    LOGGER.warning(SMART_THING_TAG,
                   "WiFi not available, skipping all network setup");
  }

  #if ENABLE_STATES
  addDeviceState("wifi", [this]() {
    return wifiConnected() ? "connected" : "disconnected";
  });
  #endif

  #if ENABLE_HOOKS 
  LOGGER.debug(SMART_THING_TAG, "Loading hooks from settings...");
  HooksManager.loadFromSettings();
  LOGGER.debug(SMART_THING_TAG, "Hooks loaded");
  #endif

  LOGGER.debug(SMART_THING_TAG, "Creating loop task");
  xTaskCreate([](void* o) { static_cast<SmartThingClass*>(o)->loopRoutine(); },
              SMART_THING_TAG, 50000, this, 1, &_loopTaskHandle);
  LOGGER.debug(SMART_THING_TAG, "Loop task created");

  #if ENABLE_LOGGER
  addConfigEntry(LOGGER_ADDRESS_CONFIG, "Logger address (ip:port)", "string");
  #endif
  // For notifications
  addConfigEntry(GATEWAY_CONFIG, "Gateway address (ip:port)", "string");

  LOGGER.debug(SMART_THING_TAG, "Setup finished");
  return true;
}

void SmartThingClass::loopRoutine() {
  const TickType_t xDelay = SMART_THING_LOOP_TASK_DELAY / portTICK_PERIOD_MS;

  for (;;) {
    if (wifiConnected()) {
      RestController.handle();
      _multicaster.broadcast(_broadcastMessage.c_str());
    }
    #if ENABLE_HOOKS 
    HooksManager.check();
    #endif
    vTaskDelay(xDelay);
  }
}

String SmartThingClass::connectToWifi() {
  if (wifiConnected()) {
    LOGGER.info(SMART_THING_TAG, "WiFi already connected");
    return WiFi.localIP().toString();
  }
  JsonObject wifiConfig = STSettings.getWiFi();
  const char* ssid = wifiConfig[SSID_SETTING];
  const char* password = wifiConfig[PASSWORD_SETTING];
  int mode = wifiConfig[WIFI_MODE_SETTING];

  if (mode == WIFI_MODE_NULL || ssid == nullptr || strlen(ssid) == 0) {
    LOGGER.warning(
        SMART_THING_TAG,
        "Ssid is blank or mode null -> creating setup AP with name %s",
        ESP.getChipModel());
    WiFi.softAP(ESP.getChipModel());
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
      _led.blink();
      while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {}
      _led.off();
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

  _led.on();
  while (!digitalRead(WIPE_PIN) &&
         millis() - started < WIPE_TIMEOUT) {
  }
  if (!digitalRead(WIPE_PIN)) {
    STSettings.dropAll();
    STSettings.save();
    LOGGER.warning(SMART_THING_TAG, "Settings were wiped!");
  }
  _led.off();
}

void SmartThingClass::updateDeviceName(String name) {
  name.trim();
  name.replace(" ", "_");
  if (name == _name) {
    return;
  }
  _name = name;
  // todo move from there
  STSettings.setDeviceName(_name.c_str());
  STSettings.save();
  updateBroadCastMessage();
  LOGGER.info(SMART_THING_TAG, "New device name %s", name.c_str());
}

void SmartThingClass::updateBroadCastMessage() {
  _broadcastMessage = _ip + "$" + _type + "$" + _name + "$" + SMART_THING_VERSION;
}

DynamicJsonDocument SmartThingClass::getConfigInfo() {
  return _configEntriesList.toJson();
}

#if ENABLE_SENSORS 
DynamicJsonDocument SmartThingClass::getSensorsValues() {
  return _sensorsList.getValues();
}

int16_t SmartThingClass::getSensorsCount() { return _sensorsList.size(); }

bool SmartThingClass::addSensor(
    const char* name,
    Configurable::ConfigurableObject<int16_t>::ValueProviderFunction function) {
  return _sensorsList.add(name, function);
}

bool SmartThingClass::addDigitalSensor(const char* name, int pin) {
  pinMode(pin, INPUT_PULLUP);
  return _sensorsList.addDigital(name, pin);
}

bool SmartThingClass::addAnalogSensor(const char* name, int pin) {
  return _sensorsList.addAnalog(name, pin);
}

const Configurable::Sensor::Sensor* SmartThingClass::getSensor(
    const char* name) {
  return _sensorsList.findSensor(name);
}
#endif

#if ENABLE_STATES
bool SmartThingClass::addDeviceState(
    const char* name,
    Configurable::ConfigurableObject<const char*>::ValueProviderFunction
        function) {
  return _deviceStatesList.add(name, function);
}

DynamicJsonDocument SmartThingClass::getDeviceStatesInfo() {
  return _deviceStatesList.getValues();
}

const Configurable::DeviceState::DeviceState* SmartThingClass::getDeviceState(
    const char* name) {
  return _deviceStatesList.findState(name);
}

int16_t SmartThingClass::getDeviceStatesCount() {
  return _deviceStatesList.size();
}

#endif

#if ENABLE_ACTIONS 
DynamicJsonDocument SmartThingClass::getActionsInfo() {
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