#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "LouverController.h"
#include "WebUtils.h"
#include "smartthing/SmartThing.h"

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35

#define AUTOMODE_SETTING "am"

LouverController controller;

void setupRestHandlers();
void processConfig();

void setup() {
    bool started = SmartThing.init("louver");
    if (started) {
        if (SmartThing.wifiConnected()) {
            setupRestHandlers();
        }
        controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN);
        controller.addLedIndicator(SmartThing.getLed());
        LOGGER.info("main", "Controller created");

        processConfig();
        LOGGER.info("main", "Config proceed");
    } else {
        LOGGER.info("main", "Failed to init smart thing");
    }

    JsonObject state = STSettings.getState();
    if (state.containsKey(AUTOMODE_SETTING) && state[AUTOMODE_SETTING].as<int>()) {
        controller.enableAutoMode();
    }
    LOGGER.info("main", "Setup finished");
}

void loop() {
    SmartThing.loopRoutine();

    if (!digitalRead(BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disableAutoMode();
        } else {
            controller.enableAutoMode();
        }
        LOGGER.statistics();
    }

    delay(500);
}

void setupRestHandlers() {
    RestController* rest = SmartThing.getRestController();

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
    JsonObject config = STSettings.getConfig();
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