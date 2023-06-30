#include "smartthing/SmartThing.h"

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

JsonArray SmartThingClass::getDeviceStates() {
    DynamicJsonDocument doc(_statesCount * 64);
    JsonArray array = doc.createNestedArray();

    DeviceState * current = _statesHead;
    while (current != nullptr) {
        JsonObject sensorObj = array.createNestedObject();
        sensorObj["name"] = current->getName();
        sensorObj["value"] = current->getValue();
        current = current->next;
    }
    return array;
}

JsonArray SmartThingClass::getSensorsValues() {
    DynamicJsonDocument doc(_sensorsCount * 64);
    JsonArray array = doc.createNestedArray();

    Sensor * currentSensor = _sensorsHead;
    while (currentSensor != nullptr) {
        JsonObject sensorObj = array.createNestedObject();
        sensorObj["name"] = currentSensor->getName();
        sensorObj["value"] = currentSensor->getValue();
        currentSensor = currentSensor->next;
    }
    return array;
}

void SmartThingClass::addDeviceState(const char * name, DeviceState::ValueGeneratorFunction function) {
    appendDeviceState(new DeviceState(name, function));
}

void SmartThingClass::appendDeviceState(DeviceState * state) {
    state->next = _statesHead;
    if (_statesHead != nullptr) {
        _statesHead->previous = state;
    }
    _statesHead = state;
    _statesCount++;
}

void SmartThingClass::addSensor(const char * name, Sensor::ValueGeneratorFunction function) {
    appendSensor(new Sensor(name, function));
}
void SmartThingClass::addDigitalSensor(const char * name, int pin) {
    pinMode(pin, INPUT);
    appendSensor(new DigitalSensor(name, pin));
}
void SmartThingClass::addAnalogSensor(const char * name, int pin) {
    appendSensor(new AnalogSensor(name, pin));
}

void SmartThingClass::appendSensor(Sensor * sensor) {
    sensor->next = _sensorsHead;
    if (_sensorsHead != nullptr) {
        _sensorsHead->previous = sensor;
    }
    _sensorsHead = sensor;
    _sensorsCount++;
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