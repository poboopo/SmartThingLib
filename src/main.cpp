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
RestHandlerResult handleAction(String actionArg);

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

    SmartThing.addDeviceState("automode", [&](){
        if (controller.isAutoModeEnabled()) {
            return "true";
        }
        return "false";
    });

    SmartThing.addAnalogSensor("light", LIGHT_SENSOR_PIN);
    SmartThing.addAnalogSensor("position", POT_PIN);
    SmartThing.addSensor("light_controller", [&]() {return controller.getLightValue();});
    SmartThing.addSensor("position_controller", [&]() {return controller.getMotorPosition();});

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

    rest->addActionHandler([rest]() {
        return handleAction(rest->getRequestArg("action"));
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


RestHandlerResult handleAction(String actionArg) {
    RestHandlerResult result;

    bool actionResult = false;

    if (!actionArg.isEmpty()) {
        int action = actionArg.toInt();
        switch(action) {
            case ENABLE_AUTO_MODE:
                actionResult = controller.enableAutoMode();
                break;
            case DISABLE_AUTO_MODE:
                actionResult = controller.disableAutoMode();
                break;
            case OPEN:
                actionResult = controller.open();
                break;
            case CLOSE:
                actionResult = controller.close();
                break;
            case MIDDLE:
                actionResult = controller.middle();
                break;
            case BRIGHT:
                actionResult = controller.bright();
                break;
            default:
                result.body = String("Wrong action ") + action;
        }
        if (!actionResult) {
            if (result.body.length() == 0) {
                result.body = "Failed to perform action";
            }
            result.code = 500;
        }
    } else {
        result.code = 400;
        result.body = "Action is missing!";
    }
    result.contentType = "text/html";
    return result;
}