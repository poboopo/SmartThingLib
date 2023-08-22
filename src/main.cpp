#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "LouverController.h"
#include "smartthing/SmartThing.h"

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35

#define AUTO_MODE_STATE "automode"
#define DISABLE_AUTO_MODE "disable_auto"
#define ENABLE_AUTO_MODE "enable_auto"
#define OPEN "open"
#define CLOSE "close"
#define MIDDLE "middle"
#define BRIGHT "bright"

#define CLOSE_SETTING "light_close"
#define OPEN_SETTING "light_open"
#define BRIGHT_SETTING "light_bright"
#define DELAY_SETTING "delay"
#define ACCURACY_SETTING "accuracy"

using namespace Configurable::Action;

LouverController controller;

void setupRestHandlers();
void processConfig();
void addDeviceStates();
void addActionsHandlers();
void registerSensors();
void addConfigEntries();
void addCallbacks();

void setup() {
    addDeviceStates();
    registerSensors();
    
    bool started = SmartThing.init("louver");
    if (started) {
        if (SmartThing.wifiConnected()) {
            setupRestHandlers();
        }
        controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN);
        controller.addLedIndicator(SmartThing.getLed());
        LOGGER.info("main", "Controller created");

        addActionsHandlers();
        addConfigEntries();
        addCallbacks();

        processConfig();
        LOGGER.info("main", "Config proceed");

        JsonObject state = STSettings.getState();
        if (state.containsKey(AUTO_MODE_STATE) && state[AUTO_MODE_STATE].as<bool>()) {
            controller.enableAutoMode();
        }
    } else {
        LOGGER.info("main", "Failed to init smart thing");
    }
    LOGGER.info("main", "Setup finished");
}

void loop() {
    SmartThing.loopRoutine();

    if (!digitalRead(WIPE_BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disableAutoMode();
        } else {
            controller.enableAutoMode();
        }
        LOGGER.statistics();
    }

    delay(200);
}

void setupRestHandlers() {
    RestController* rest = SmartThing.getRestController();

    rest->addConfigUpdatedHandler([]() {
        processConfig();
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
}

void addConfigEntries() {
    SmartThing.addConfigEntry(DELAY_SETTING, "Automode update delay", "number");
    SmartThing.addConfigEntry(ACCURACY_SETTING, "Motor accuracy", "number");
    SmartThing.addConfigEntry(CLOSE_SETTING, "Light close value", "number");
    SmartThing.addConfigEntry(OPEN_SETTING, "Light open value", "number");
    SmartThing.addConfigEntry(BRIGHT_SETTING, "Light bright value", "number");
}

void addDeviceStates() {
    SmartThing.addDeviceState("automode", [](){
        if (controller.isAutoModeEnabled()) {
            return "true";
        }
        return "false";
    });
    SmartThing.addDeviceState("position", []() {
        return controller.getPosition();
    });
    SmartThing.addDeviceState("led", []() {
        return SmartThing.getLed()->isOn() ? "on" : "off";
    });
}

void registerSensors() {
    SmartThing.registerAnalogSensor("light", LIGHT_SENSOR_PIN);
    SmartThing.registerSensor("position", []() {return controller.getMotorPosition();});

    SmartThing.registerDigitalSensor("test_digital", 12);
}

void addCallbacks() {
    SmartThing.getCallbacksManager()->addDeviceStateCallback("automode", [](String * value) {
        LOGGER.info("main", "Automode callback called. New value %s", *value);
        JsonObject state = STSettings.getState();
        state[AUTO_MODE_STATE] = value->c_str();
        STSettings.save();
    });
    SmartThing.getCallbacksManager()->addSensorCallback("test_digital", [](int16_t * value) {
        LOGGER.debug("main", "Digital sensor value changed to %u", *value);
        if (controller.isAutoModeEnabled()) {
            controller.disableAutoMode();
        } else {
            controller.enableAutoMode();
        }
    }, 1);
}

void addActionsHandlers() {
    SmartThing.addActionHandler(ENABLE_AUTO_MODE, "Enable automode", [](){
        return ActionResult(controller.enableAutoMode());
    });
    SmartThing.addActionHandler(DISABLE_AUTO_MODE, "Disable automode", [](){
        return ActionResult(controller.disableAutoMode());
    });
    SmartThing.addActionHandler(OPEN, "Set louver open position", [](){
        return ActionResult(controller.open());
    });
    SmartThing.addActionHandler(CLOSE, "Set louver close position", [](){
        return ActionResult(controller.close());
    });
    SmartThing.addActionHandler(MIDDLE, "Set louver middle position", [](){
        return ActionResult(controller.middle());
    });
    SmartThing.addActionHandler(BRIGHT, "Set louver bright position", [](){
        return ActionResult(controller.bright());
    });
}