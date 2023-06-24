#include "SmartThing.h"

SmartThing::SmartThing() {};
SmartThing::~SmartThing() {};
String SmartThing::_type = "no_type";
String SmartThing::_name = "no_name";

bool SmartThing::wifiConnected() {
    return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

String buildBroadCastMessage(String ip, String name) {
    return ip + ":" + name;
}

bool SmartThing::init(String type) {
    BetterLogger::init();
    BetterLogger::log(SMART_THING_TAG, "Smart thing initialization started");

    SettingsManager::loadSettings();
    BetterLogger::log(SMART_THING_TAG, "Settings manager loaded");

    SmartThing::_type = type;
    SmartThing::_name = SettingsManager::getSettingString(DEVICE_NAME);
    BetterLogger::log(SMART_THING_TAG, "Device type/name: %s/%s", SmartThing::_type, SmartThing::_name);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    _led.init(LED_PIN);
    
    if (!digitalRead(BUTTON_PIN)) {
        wipeSettings();
    }

    String ssid = SettingsManager::getSettingString(GROUP_WIFI, SSID_SETTING);
    String password = SettingsManager::getSettingString(GROUP_WIFI, PASSWORD_SETTING);
    int mode = SettingsManager::getSettingInteger(GROUP_WIFI, WIFI_MODE_SETTING);
    _ip = connectToWifi(ssid, password, mode);

    if (wifiConnected()) {
        BetterLogger::connect(_ip.c_str(), SmartThing::_name.c_str());
        BetterLogger::log(SMART_THING_TAG, "WiFi connected, local ip %s", _ip);

        ArduinoOTA.begin();
        BetterLogger::log(SMART_THING_TAG, "Ota started");

        _multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
        _broadcastMessage = buildBroadCastMessage(_ip, ESP.getChipModel());
        BetterLogger::log(SMART_THING_TAG, "Multicaster created");

        _rest.addWifiupdatedHandler([this](){
            BetterLogger::log(SMART_THING_TAG, "WiFi updated, reloading wifi!");
            WiFi.disconnect();
            WiFi.mode(WIFI_MODE_NULL);
            delay(500);
            String ssid = SettingsManager::getSettingString(GROUP_WIFI, SSID_SETTING);
            String password = SettingsManager::getSettingString(GROUP_WIFI, PASSWORD_SETTING);
            int mode = SettingsManager::getSettingInteger(GROUP_WIFI, WIFI_MODE_SETTING);
            _ip = connectToWifi(ssid, password, mode);
            BetterLogger::log(SMART_THING_TAG, "WiFi reloaded");
        });
        _rest.begin();
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
        SettingsManager::dropAll();
        SettingsManager::saveSettings();
        BetterLogger::log(SMART_THING_TAG, "Settings were wiped!");
    }
    _led.off();
}

const String SmartThing::getType() {
    return SmartThing::_type;
}

const String SmartThing::getName() {
    return SmartThing::_name;
}

void SmartThing::setName(String name) {
    _name = name;
    SettingsManager::putSetting(DEVICE_NAME, name);
    SettingsManager::saveSettings();
    // todo
    // _broadcastMessage = buildBroadCastMessage(_ip, _name);
}

RestController* SmartThing::getRestController() {
    return &_rest;
}

LedIndicator* SmartThing::getLed() {
    return &_led;
}