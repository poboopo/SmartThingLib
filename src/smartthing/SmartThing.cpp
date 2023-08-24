#include "smartthing/SmartThing.h"
#include "smartthing/utils/StringUtils.h"

using namespace Configurable;

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

    pinMode(WIPE_BUTTON_PIN, INPUT_PULLUP);
    _led.init(LED_PIN);
    
    if (!digitalRead(WIPE_BUTTON_PIN)) {
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
            _ip = connectToWifi();
            LOGGER.info(SMART_THING_TAG, "WiFi reloaded");
            LOGGER.info(SMART_THING_TAG, "Reloading rest...");
            _rest.reload();
            LOGGER.info(SMART_THING_TAG, "Rest reloaded");
        });
        _rest.begin();
        LOGGER.debug(SMART_THING_TAG, "RestController started");
    } else {
        LOGGER.warning(SMART_THING_TAG, "WiFi not available, skipping all network setup");
    }

    addDeviceState("wifi", [this]() {
        return wifiConnected() ? "connected" : "disconnected";
    });

    LOGGER.debug(SMART_THING_TAG, "Loading callbacks from settings...");
    _callbacksManager.loadFromSettings();
    LOGGER.debug(SMART_THING_TAG, "Callbacks loaded");

    LOGGER.debug(SMART_THING_TAG, "Creating loop task");
    xTaskCreate(
        [](void* o){ static_cast<SmartThingClass*>(o)->loopRoutine(); },
        SMART_THING_TAG,
        20000,
        this,
        1,
        &_loopTaskHandle
    );
    LOGGER.debug(SMART_THING_TAG, "Loop task created");

    LOGGER.debug(SMART_THING_TAG, "Setup finished");
    return true;
}

void SmartThingClass::loopRoutine() {
    const TickType_t xDelay = SMART_THING_LOOP_TASK_DELAY / portTICK_PERIOD_MS;

    for(;;) {
        if (wifiConnected()) {
            ArduinoOTA.handle();
            _rest.handle();
            _multicaster.broadcast(_broadcastMessage.c_str());
        }
        _callbacksManager.check();
        vTaskDelay(xDelay);
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
    while (!digitalRead(WIPE_BUTTON_PIN) && millis() - started < WIPE_BUTTON_TIME) {}
    if (!digitalRead(WIPE_BUTTON_PIN)) {
        STSettings.dropAll();
        STSettings.save();
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
    STSettings.save();
    _broadcastMessage = buildBroadCastMessage(_ip, _name);
    LOGGER.info(SMART_THING_TAG, "New device name %s", name.c_str());
}

DynamicJsonDocument SmartThingClass::getInfoDictionaries() {
    int size = _actionsList.size() + _configEntriesList.size();

    DynamicJsonDocument doc(size * 64);
    doc["actions"] = _actionsList.toJson();
    doc["config"] = _configEntriesList.toJson();
    return doc;
}

DynamicJsonDocument SmartThingClass::getDeviceStatesInfo() {
    return _deviceStatesList.getValues();
}

DynamicJsonDocument SmartThingClass::getSensorsValues() {
    return _sensorsList.getValues();
}

DynamicJsonDocument SmartThingClass::getActionsInfo() {
    return _actionsList.toJson();
}

DynamicJsonDocument SmartThingClass::getConfigInfo() {
    return _configEntriesList.toJson();
}

DynamicJsonDocument SmartThingClass::getWatchersInfo() {
    return _callbacksManager.getWatchersInfo();
}

// add possible values?
bool SmartThingClass::addDeviceState(const char * name, Configurable::ConfigurableObject<const char *>::ValueGeneratorFunction function) {
    return _deviceStatesList.add(name, function);
}

bool SmartThingClass::registerSensor(const char * name, Configurable::ConfigurableObject<int16_t>::ValueGeneratorFunction function) {
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

const Configurable::DeviceState::DeviceState * SmartThingClass::getDeviceState(const char * name) {
    return _deviceStatesList.findState(name);
}

const Configurable::Sensor::Sensor * SmartThingClass::getSensor(const char * name) {
    return _sensorsList.findSensor(name);
}

const String SmartThingClass::getType() {
    return SmartThingClass::_type;
}

const String SmartThingClass::getName() {
    return SmartThingClass::_name;
}