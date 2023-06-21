#include "SmartThing.h"

SmartThing::SmartThing() {};
SmartThing::~SmartThing() {};

bool SmartThing::wifiConnected() {
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
    int mode = _settingsManager.getSettingInteger(GROUP_WIFI, WIFI_MODE_SETTING);
    _ip = connectToWifi(ssid, password, mode);

    if (wifiConnected()) {
        BetterLogger::connect(_ip.c_str(), _name.c_str());
        BetterLogger::log(SMART_THING_TAG, "WiFi connected, local ip %s", _ip);

        ArduinoOTA.begin();
        BetterLogger::log(SMART_THING_TAG, "Ota started");

        _multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
        _broadcastMessage = buildBroadCastMessage(_ip, ESP.getChipModel());
        BetterLogger::log(SMART_THING_TAG, "Multicaster created");

        _rest.addGetInfoHandler([this](){
            RestHandlerResult result;
            result.code = 200;
            result.contentType = JSON_CONTENT_TYPE;
            result.body = buildInfoJson();
            return result;
        });
        _rest.addWifiupdatedHandler([this](){
            BetterLogger::log(SMART_THING_TAG, "WiFi updated, reloading wifi!");
            WiFi.disconnect();
            WiFi.mode(WIFI_MODE_NULL);
            delay(500);
            String ssid = _settingsManager.getSettingString(GROUP_WIFI, SSID_SETTING);
            String password = _settingsManager.getSettingString(GROUP_WIFI, PASSWORD_SETTING);
            int mode = _settingsManager.getSettingInteger(GROUP_WIFI, WIFI_MODE_SETTING);
            _ip = connectToWifi(ssid, password, mode);
            BetterLogger::log(SMART_THING_TAG, "WiFi reloaded");
        });
        _rest.begin(&_settingsManager);
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

String SmartThing::connectToWifi(String ssid, String password, int mode) {
    if (wifiConnected()) {
        BetterLogger::log(SMART_THING_TAG, "WiFi already connected");
        return WiFi.localIP().toString();
    }

    if (ssid.isEmpty()) {
        BetterLogger::log(SMART_THING_TAG, "Ssid is blank -> creating setup AP with name %s", ESP.getChipModel());
        WiFi.softAP(ESP.getChipModel());
        delay(500);
        return WiFi.softAPIP().toString();
    } else {
        if (mode == WIFI_MODE_AP) {
            BetterLogger::log(SMART_THING_TAG, "Creating AP point %s :: %s", ssid, password);
            if (password.isEmpty()) {
                WiFi.softAP(ssid.c_str(), password.c_str());
            } else {
                WiFi.softAP(ssid.c_str());
            }
            delay(500);
            BetterLogger::log(SMART_THING_TAG, "WiFi started in AP mode");
            return WiFi.softAPIP().toString();
        } else if (mode == WIFI_MODE_STA) {
            BetterLogger::log(SMART_THING_TAG, "WiFi connecting to %s :: %s", ssid, password);
            WiFi.begin(ssid.c_str(), password.c_str());
            long startTime = millis();
            _led.blink();
            while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {}
            _led.off();
            if (WiFi.isConnected()) {
                BetterLogger::log(SMART_THING_TAG, "WiFi started in STA mode");
                return WiFi.localIP().toString();
            } else {
                WiFi.disconnect();
                return "";
            }
        } else {
            BetterLogger::log(SMART_THING_TAG, "Mode %d not sipported!", mode);
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