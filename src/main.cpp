#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "LouverController.h"
#include "net/WebUtils.h"
#include "SmartThing.h"

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35

LouverController controller;
SmartThing smartThing;

void setupRestHandlers();
void processConfig();

void setup() {
    if (smartThing.init("louver")) {
        if (smartThing.wifiConnected()) {
            setupRestHandlers();
        }
        controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN);
        controller.addLedIndicator(smartThing.getLed());
        BetterLogger::log("main", "Controller created");

        processConfig();
        BetterLogger::log("main", "Config proceed");
    } else {
        BetterLogger::log("main", "Failed to init smart thing");
    }
    BetterLogger::log("main", "Setup finished");
}

void loop() {
    smartThing.loopRoutine();

    if (!digitalRead(BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disableAutoMode();
        } else {
            controller.enableAutoMode();
        }
        BetterLogger::statistics();
    }

    delay(500);
}

void setupRestHandlers() {
    RestController* rest = smartThing.getRestController();

    rest->addGetStateHandler([]() {
        RestHandlerResult result = getLouverStateJson(&controller);
        return result;
    });
    rest->addActionHandler([rest]() {
        return handleAction(rest->getRequestArg("action") ,&controller);
    });
    rest->addGetSensorsHandler([](){
        RestHandlerResult result = getSensorsJson(&controller);
        return result;
    });
    rest->addConfigUpdatedHandler([]() {
        processConfig();
    });
    rest->addGetDictsHandler([](){
        RestHandlerResult result = getDictionaries();
        return result;
    });
}

void processConfig() {
    SettingsManager* settingsManager = smartThing.getSettingsManager();

    if (settingsManager->getSettingInteger(GROUP_STATE, AUTOMODE_SETTING)) {
        controller.enableAutoMode();
    }

    JsonObject config = settingsManager->getSettings(GROUP_CONFIG);
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