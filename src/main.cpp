#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

#include "LouverController.h"
#include "net/BetterLogger.h"
#include "net/Multicaster.h"
#include "net/RestController.h"
#include "utils/SettingsManager.h"
#include "utils/LedIndicator.h"
#include "net/WebUtils.h"

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35
#define LED_PIN 5
#define BUTTON_PIN 18

#define WIFI_SETUP_TIMEOUT 10000
#define WIPE_BUTTON_TIME 5000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

LouverController controller;
Multicaster multicaster;
SettingsManager settingsManager;
LedIndicator ledIndicator;
BetterLogger logger;
RestController rest;

String myIp;

void setupRestHandlers();
String connectToWifi();
void wipeSettings();
void processConfig();

bool wifiConnected() {
    return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

void setup() {
    logger.log("*", "Setup started");
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    ledIndicator.init(LED_PIN);
    settingsManager.addLogger(&logger);
    settingsManager.loadSettings();
    logger.log("*", "Settings manager loaded");
    
    if (!digitalRead(BUTTON_PIN)) {
        wipeSettings();
    }

    myIp = connectToWifi();

    if (wifiConnected()) {
        logger.connect(myIp.c_str());
        logger.log("*", "WiFi connected, local ip %s", myIp);

        ArduinoOTA.begin();
        logger.log("*", "Ota started");

        multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
        logger.log("*", "Multicaster created");

        setupRestHandlers();
        rest.begin(&logger, &settingsManager);
        logger.log("*", "RestController started");
    } else {
        logger.log("*", "WiFi not available, skipping all network setup");
    }

    controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN);
    controller.addLedIndicator(&ledIndicator);
    controller.addLogger(&logger);
    if (settingsManager.getSettingInteger(GROUP_STATE, AUTOMODE_SETTING)) {
        controller.enableAutoMode();
    }
    logger.log("*", "Controller created");

    processConfig();
    logger.log("*", "Config proceed");

    logger.log("*", "Setup finished");
}

void loop() {
    if (wifiConnected()) {
        ArduinoOTA.handle();
        rest.handle();
        multicaster.broadcast(myIp.c_str());
    }

    if (!digitalRead(BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disableAutoMode();
        } else {
            controller.enableAutoMode();
        }
        logger.statistics();
    }

    delay(500);
}

void wipeSettings() {
    long started = millis();
    ledIndicator.on();
    while (!digitalRead(BUTTON_PIN) && millis() - started < WIPE_BUTTON_TIME) {
    }
    ledIndicator.off();

    if (!digitalRead(BUTTON_PIN)) {
        settingsManager.dropAll();
        settingsManager.saveSettings();
        logger.log("*", "Settings were droped!");
    }
}

String connectToWifi() {
    String ssid = settingsManager.getSettingString(GROUP_WIFI, SSID_SETTING);
    String password = settingsManager.getSettingString(GROUP_WIFI, PASSWORD_SETTING);
    
    if (ssid.isEmpty()) {
        logger.log("*", "Ssid is blank -> creating AP");
        WiFi.softAP("LOUVER");
        // WiFi.beginSmartConfig();
        delay(500);

        if (MDNS.begin("louver")) {
            MDNS.addService("http", "tcp", 80);
        } else {
            logger.log("*", "Failed to setup up MDNS");
        }

        return WiFi.softAPIP().toString();
    } else {
        logger.log("*", "WiFi connecting to %s :: %s", ssid.c_str(), password.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        long startTime = millis();
        ledIndicator.blink();
        while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {}
        ledIndicator.off();
        if (WiFi.isConnected()) {
            return WiFi.localIP().toString();
        } else {
            WiFi.disconnect();
            return "";
        }
    }
}

void setupRestHandlers() {
    rest.addGetStateHandler([]() {
        HandlerResult result = getLouverStateJson(&controller);
        return result;
    });
    rest.addStateChangeHandler([]() {
        return changeLouverState(rest.getRequestBody() ,&controller);
    });

    rest.addConfigUpdatedHandler([]() {
        processConfig();
    });

    rest.addWebPageBuilder([](){
        HandlerResult result;
        result.body = buildMainPage(WiFi.getMode() == WIFI_AP);
        result.contentType = "text/html";
        return result;
    });

    if (WiFi.getMode() == WIFI_MODE_AP) {
        rest.addSetupEndpoint();    
    }
}

// TODO вынести нафиг отседова
void processConfig() {
    JsonObject config = settingsManager.getSettings(GROUP_CONFIG);

    int lightClose = config[CLOSE_SETTING];
    int lightOpen = config[OPEN_SETTING];
    int lightBright = config[BRIGHT_SETTING];
    if (lightClose != 0 && lightOpen != 0 && lightBright != 0) {
        controller.setLightValues(lightClose, lightOpen, lightBright);
    }

    int delay = config[DELAY_SETTING];
    if (delay > 0) {
        controller.setMonitorTaskDelay(delay);
    }

    uint8_t accuracy = config[ACCURACY_SETTING];
    if (accuracy > 0) {
        controller.setMotorAccuracy(accuracy);
    }

    controller.restartAutoMode();
}