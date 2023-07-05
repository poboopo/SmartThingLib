#include "smartthing/SmartThing.h"

using namespace Configurable;
using namespace Watcher;

SmartThingClass SmartThing;

SmartThingClass::SmartThingClass() {};
SmartThingClass::~SmartThingClass() {};

bool SmartThingClass::wifiConnected() {
    return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

String buildBroadCastMessage(String ip, String name) {
    return ip + ":" + name;
}

bool SmartThingClass::init(String type) {
    LOGGER.init();
    LOGGER.debug(SMART_THING_TAG, "Smart thing initialization started");

    STSettings.loadSettings();
    LOGGER.debug(SMART_THING_TAG, "Settings manager loaded");

    _type = type;
    _name = STSettings.getDeviceName();
    if (_name.isEmpty()) {
        _name = ESP.getChipModel();
        STSettings.setDeviceName(_name.c_str());
    }
    LOGGER.debug(SMART_THING_TAG, "Device type/name: %s/%s", _type, _name);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    _led.init(LED_PIN);
    
    if (!digitalRead(BUTTON_PIN)) {
        wipeSettings();
    }

    _ip = connectToWifi();

    if (wifiConnected()) {
        LOGGER.connect(_ip.c_str(), _name.c_str());
        LOGGER.info(SMART_THING_TAG, "WiFi connected, local ip %s", _ip);

        ArduinoOTA.begin();
        LOGGER.debug(SMART_THING_TAG, "Ota started");

        _multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
        _broadcastMessage = buildBroadCastMessage(_ip, _name.c_str());
        LOGGER.debug(SMART_THING_TAG, "Multicaster created");

        _rest.addWifiupdatedHandler([&](){
            LOGGER.warning(SMART_THING_TAG, "WiFi updated, reloading wifi!");
            WiFi.disconnect();
            WiFi.mode(WIFI_MODE_NULL);
            delay(500);
            _ip = connectToWifi();\
            // todo reload rest
            LOGGER.info(SMART_THING_TAG, "WiFi reloaded");
        });
        _rest.begin();
        LOGGER.debug(SMART_THING_TAG, "RestController started");
    } else {
        LOGGER.warning(SMART_THING_TAG, "WiFi not available, skipping all network setup");
    }

    LOGGER.debug(SMART_THING_TAG, "Setup finished");
    return true;
}

void SmartThingClass::loopRoutine() {
    if (wifiConnected()) {
        ArduinoOTA.handle();
        _rest.handle();
        _multicaster.broadcast(_broadcastMessage.c_str());
        _watchersList.check();
    }
}

String SmartThingClass::connectToWifi() {
    if (wifiConnected()) {
        LOGGER.info(SMART_THING_TAG, "WiFi already connected");
        return WiFi.localIP().toString();
    }
    JsonObject wifiConfig = STSettings.getWiFi();
    const char * ssid = wifiConfig[SSID_SETTING];
    const char * password = wifiConfig[PASSWORD_SETTING];
    int mode = wifiConfig[WIFI_MODE_SETTING];

    if (mode == WIFI_MODE_NULL || ssid == nullptr || strlen(ssid) == 0) {
        LOGGER.warning(SMART_THING_TAG, "Ssid is blank or mode null -> creating setup AP with name %s", ESP.getChipModel());
        WiFi.softAP(ESP.getChipModel());
        delay(500);
        LOGGER.info(SMART_THING_TAG, "WiFi started in soft AP mode");
        return WiFi.softAPIP().toString();
    } else {
        if (mode == WIFI_MODE_AP) {
            if (password != nullptr && strlen(password) >= 0) {
                LOGGER.info(SMART_THING_TAG, "Creating AP point %s :: %s", ssid, password);
                WiFi.softAP(ssid, password);
            } else {
                LOGGER.info(SMART_THING_TAG, "Creating AP point %s", ssid);
                WiFi.softAP(ssid);
            }
            delay(500);
            LOGGER.info(SMART_THING_TAG, "WiFi started in AP mode");
            return WiFi.softAPIP().toString();
        } else if (mode == WIFI_MODE_STA) {
            LOGGER.debug(SMART_THING_TAG, "WiFi connecting to %s :: %s", ssid, password);
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
    LOGGER.warning(SMART_THING_TAG, "ALL SETTINGS WILL BE WIPED IN %d ms!!!", WIPE_BUTTON_TIME);

    _led.on();
    while (!digitalRead(BUTTON_PIN) && millis() - started < WIPE_BUTTON_TIME) {}
    if (!digitalRead(BUTTON_PIN)) {
        STSettings.dropAll();
        LOGGER.warning(SMART_THING_TAG, "Settings were wiped!");
    }
    _led.off();
}

void SmartThingClass::setName(String name) {
    if (name == _name) {
        return;
    }
    _name = name;
    // todo move from there
    STSettings.setDeviceName(name.c_str());
    STSettings.saveSettings();
    _broadcastMessage = buildBroadCastMessage(_ip, _name);
}

DynamicJsonDocument SmartThingClass::getDictionaries() {
    int size = _actionsList.size() + _configEntriesList.size();

    DynamicJsonDocument doc(size * 64);
    doc["actions"] = _actionsList.getDict();
    doc["config"] = _configEntriesList.getDict();
    return doc;
}

DynamicJsonDocument SmartThingClass::getDeviceStates() {
    return _deviceStatesList.getValues();
}

DynamicJsonDocument SmartThingClass::getSensorsValues() {
    return _sensorsList.getValues();
}

DynamicJsonDocument SmartThingClass::getActionsDict() {
    return _actionsList.getDict();
}

DynamicJsonDocument SmartThingClass::getConfigEntriesDict() {
    return _configEntriesList.getDict();
}

bool SmartThingClass::addDeviceState(const char * name, DeviceState::ValueGeneratorFunction function) {
    return _deviceStatesList.add(name, function);
}

bool SmartThingClass::registerSensor(const char * name, Sensor::ValueGeneratorFunction function) {
    return _sensorsList.add(name, function);
}
bool SmartThingClass::registerDigitalSensor(const char * name, int pin) {
    pinMode(pin, INPUT_PULLUP);
    return _sensorsList.addDigital(name, pin);
}
bool SmartThingClass::registerAnalogSensor(const char * name, int pin) {
    return _sensorsList.addAnalog(name, pin);
}

bool SmartThingClass::addActionHandler(const char * action, const char * caption, Action::ActionHandler handler) {
    return _actionsList.add(action, caption, handler);
}

Action::ActionResult SmartThingClass::callAction(const char * action) {
    return _actionsList.callAction(action);
}

bool SmartThingClass::addConfigEntry(const char * name, const char * caption, const char * type) {
    return _configEntriesList.add(name, caption, type);
}

bool SmartThingClass::registerSensorWatcher(const char * name, Callback::LambdaCallback<uint16_t>::CustomCallback callback) {
    const Sensor::Sensor * sensor = _sensorsList.findSensor(name);
    if (sensor == nullptr) {
        LOGGER.error(SMART_THING_TAG, "Can't find sensor with name %s. Not registered yet?", name);
        return false;
    }
    Callback::LambdaCallback<uint16_t> * watcherCallback = new Callback::LambdaCallback<uint16_t>(callback);
    return _watchersList.registerSensorWatcher(sensor, watcherCallback);
}

bool SmartThingClass::registerSensorWatcher(const char * name, const char * url) {
    const Sensor::Sensor * sensor = _sensorsList.findSensor(name);
    if (sensor == nullptr) {
        LOGGER.error(SMART_THING_TAG, "Can't find sensor with name %s. Not registered yet?", name);
        return false;
    }
    Callback::HttpCallback<uint16_t> * watcherCallback = new Callback::HttpCallback<uint16_t>(url);
    return _watchersList.registerSensorWatcher(sensor, watcherCallback);
}

bool SmartThingClass::registerDeviceStateWatcher(const char * name, Callback::LambdaCallback<const char *>::CustomCallback callback) {
    const DeviceState::DeviceState * state = _deviceStatesList.findState(name);
    if (state == nullptr) {
        LOGGER.error(SMART_THING_TAG, "Can't find device state with name %s. Not registered yet?", name);
        return false;
    }
    Callback::LambdaCallback<const char *> * watcherCallback = new Callback::LambdaCallback<const char *>(callback);
    return _watchersList.registerDeviceStateWatcher(state, watcherCallback);
}
bool SmartThingClass::registerDeviceStateWatcher(const char * name, const char * url) {
    const DeviceState::DeviceState * state = _deviceStatesList.findState(name);
    if (state == nullptr) {
        LOGGER.error(SMART_THING_TAG, "Can't find device state with name %s. Not registered yet?", name);
        return false;
    }
    Callback::HttpCallback<const char *> * watcherCallback = new Callback::HttpCallback<const char *>(url);
    return _watchersList.registerDeviceStateWatcher(state, watcherCallback);
}

const String SmartThingClass::getType() {
    return SmartThingClass::_type;
}

const String SmartThingClass::getName() {
    return SmartThingClass::_name;
}

RestController* SmartThingClass::getRestController() {
    return &_rest;
}

LedIndicator* SmartThingClass::getLed() {
    return &_led;
}