#include "SmartThing.h"

SmartThing::SmartThing() {};
SmartThing::~SmartThing() {};

bool wifiConnected() {
    return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

String buildBroadCastMessage(String ip, String name) {
    return ip + ":" + name;
}

bool SmartThing::init(String type) {
    _type = type;
    BetterLogger::init();
    BetterLogger::log(SMART_THING_TAG, "Smart thing initialization started");

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    _led.init(LED_PIN);
    
    _settingsManager.loadSettings();
    BetterLogger::log(SMART_THING_TAG, "Settings manager loaded");
    if (!digitalRead(BUTTON_PIN)) {
        wipeSettings();
    }

    String ssid = _settingsManager.getSettingString(GROUP_WIFI, SSID_SETTING);
    String password = _settingsManager.getSettingString(GROUP_WIFI, PASSWORD_SETTING);
    _ip = connectToWifi(ssid, password);

    if (wifiConnected()) {
        BetterLogger::connect(_ip.c_str(), _name.c_str());
        BetterLogger::log(SMART_THING_TAG, "WiFi connected, local ip %s", _ip);

        ArduinoOTA.begin();
        BetterLogger::log(SMART_THING_TAG, "Ota started");

        _multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
        _broadcastMessage = buildBroadCastMessage(_ip, ESP.getChipModel());
        BetterLogger::log(SMART_THING_TAG, "Multicaster created");

        _rest.begin(&_settingsManager);
        _rest.addGetInfoHandler([&](){
            HandlerResult result;
            result.code = 200;
            result.contentType = JSON_CONTENT_TYPE;
            result.body = buildInfoJson();
            return result;
        });

        if (WiFi.getMode() == WIFI_MODE_AP) {
            _rest.addSetupEndpoint();    
        }
        BetterLogger::log(SMART_THING_TAG, "RestController started");
    } else {
        BetterLogger::log(SMART_THING_TAG, "WiFi not available, skipping all network setup");
    }

    BetterLogger::log(SMART_THING_TAG, "Setup finished");
    return true;
}

void SmartThing::loopRoutine() {
    if (wifiConnected()) {
        ArduinoOTA.handle();
        _rest.handle();
        _multicaster.broadcast(_broadcastMessage.c_str());
    }
}

String SmartThing::connectToWifi(String ssid, String password) {
    if (ssid.isEmpty()) {
        BetterLogger::log(SMART_THING_TAG, "Ssid is blank -> creating AP");
        WiFi.softAP(ESP.getChipModel());
        delay(500);
        return WiFi.softAPIP().toString();
    } else {
        BetterLogger::log(SMART_THING_TAG, "WiFi connecting to %s :: %s", ssid.c_str(), password.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        long startTime = millis();
        _led.blink();
        while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {}
        _led.off();
        if (WiFi.isConnected()) {
            return WiFi.localIP().toString();
        } else {
            WiFi.disconnect();
            return "";
        }
    }
}

void SmartThing::wipeSettings() {
    long started = millis();
    BetterLogger::log(SMART_THING_TAG, "ALL SETTINGS WILL BE WIPED IN %d ms!!!", WIPE_BUTTON_TIME);

    _led.on();
    while (!digitalRead(BUTTON_PIN) && millis() - started < WIPE_BUTTON_TIME) {}
    if (!digitalRead(BUTTON_PIN)) {
        _settingsManager.dropAll();
        _settingsManager.saveSettings();
        BetterLogger::log(SMART_THING_TAG, "Settings were wiped!");
    }
    _led.off();
}


String SmartThing::buildInfoJson() {
    DynamicJsonDocument jsonDoc(256);
    jsonDoc["version"] = SMART_THING_VERSION;
    jsonDoc["name"] = _name;
    jsonDoc["type"] = _type;
    jsonDoc["chip_model"] = ESP.getChipModel();
    jsonDoc["chip_revision"] = ESP.getChipRevision();

    String result;
    serializeJson(jsonDoc, result);
    return result;
}

void SmartThing::setName(String name) {
    _name = name;
    _broadcastMessage = buildBroadCastMessage(_ip, _name);
}

RestController* SmartThing::getRestController() {
    return &_rest;
}

SettingsManager* SmartThing::getSettingsManager() {
    return &_settingsManager;
}

LedIndicator* SmartThing::getLed() {
    return &_led;
}