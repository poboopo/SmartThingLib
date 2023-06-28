#include "SmartThing.h"

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

    SettingsManager::loadSettings();
    LOGGER.debug(SMART_THING_TAG, "Settings manager loaded");

    _type = type;
    _name = SettingsManager::getSettingString(DEVICE_NAME);
    LOGGER.debug(SMART_THING_TAG, "Device type/name: %s/%s", _type, _name);

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
            String ssid = SettingsManager::getSettingString(GROUP_WIFI, SSID_SETTING);
            String password = SettingsManager::getSettingString(GROUP_WIFI, PASSWORD_SETTING);
            int mode = SettingsManager::getSettingInteger(GROUP_WIFI, WIFI_MODE_SETTING);
            _ip = connectToWifi(ssid, password, mode);
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

String SmartThingClass::connectToWifi(String ssid, String password, int mode) {
    if (wifiConnected()) {
        LOGGER.info(SMART_THING_TAG, "WiFi already connected");
        return WiFi.localIP().toString();
    }

    if (ssid.isEmpty()) {
        LOGGER.warning(SMART_THING_TAG, "Ssid is blank -> creating setup AP with name %s", ESP.getChipModel());
        WiFi.softAP(ESP.getChipModel());
        delay(500);
        LOGGER.info(SMART_THING_TAG, "WiFi started in soft AP mode");
        return WiFi.softAPIP().toString();
    } else {
        if (mode == WIFI_MODE_AP) {
            LOGGER.debug(SMART_THING_TAG, "Creating AP point %s :: %s", ssid, password);
            if (password.isEmpty()) {
                WiFi.softAP(ssid.c_str(), password.c_str());
            } else {
                WiFi.softAP(ssid.c_str());
            }
            delay(500);
            LOGGER.info(SMART_THING_TAG, "WiFi started in AP mode");
            return WiFi.softAPIP().toString();
        } else if (mode == WIFI_MODE_STA) {
            LOGGER.debug(SMART_THING_TAG, "WiFi connecting to %s :: %s", ssid, password);
            WiFi.begin(ssid.c_str(), password.c_str());
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
        SettingsManager::dropAll();
        SettingsManager::saveSettings();
        LOGGER.warning(SMART_THING_TAG, "Settings were wiped!");
    }
    _led.off();
}

const String SmartThingClass::getType() {
    return SmartThingClass::_type;
}

const String SmartThingClass::getName() {
    return SmartThingClass::_name;
}

void SmartThingClass::setName(String name) {
    _name = name;
    SettingsManager::putSetting(DEVICE_NAME, name);
    SettingsManager::saveSettings();
    // todo
    // _broadcastMessage = buildBroadCastMessage(_ip, _name);
}

RestController* SmartThingClass::getRestController() {
    return &_rest;
}

LedIndicator* SmartThingClass::getLed() {
    return &_led;
}