#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include <net/Pages.h>
#include <ArduinoJson.h>
#include <utils/SettingsManager.h>
#include <net/RestController.h>
#include <LouverController.h>

//TODO MOVE THIS FILE TO /utils

#define CLOSE_SETTING "light_close"
#define OPEN_SETTING "light_open"
#define BRIGHT_SETTING "light_bright"
#define DELAY_SETTING "delay"
#define ACCURACY_SETTING "accuracy"
#define NAME_SETTING "name"

enum LOUVER_ACTIONS {
    DISABLE_AUTO_MODE,
    ENABLE_AUTO_MODE,
    OPEN,
    CLOSE,
    MIDDLE,
    BRIGHT
};

HandlerResult getLouverStateJson(LouverController * controller) {
    DynamicJsonDocument jsonDoc(64);
    jsonDoc["automode"] = controller->isAutoModeEnabled();
    jsonDoc["position"] = controller->getMotorPosition();
    jsonDoc["light"] = controller->getLightValue();

    HandlerResult result;
    result.code = 200;
    result.contentType = JSON_CONTENT_TYPE;
    serializeJson(jsonDoc, result.body);
    return result;
}

HandlerResult getSensorsJson(LouverController * controller) {
    DynamicJsonDocument jsonDoc(64);
    jsonDoc["light"] = controller->getLightValue();
    jsonDoc["temp"] = 10;

    HandlerResult result;
    result.code = 200;
    result.contentType = JSON_CONTENT_TYPE;
    serializeJson(jsonDoc, result.body);
    return result;
}

HandlerResult getDictionaries() {
    DynamicJsonDocument jsonDoc(1024);

    JsonArray actions = jsonDoc.createNestedArray("actions");
    JsonObject disableAuto = actions.createNestedObject();
    disableAuto["caption"] = "Disable automode";
    disableAuto["id"] = DISABLE_AUTO_MODE;
    JsonObject enableAuto = actions.createNestedObject();
    enableAuto["caption"] = "Enable automode";
    enableAuto["id"] = ENABLE_AUTO_MODE;
    JsonObject openAction = actions.createNestedObject();
    openAction["caption"] = "Set louver open position";
    openAction["id"] = OPEN;
    JsonObject closeAction = actions.createNestedObject();
    closeAction["caption"] = "Set louver close position";
    closeAction["id"] = CLOSE;
    JsonObject middleAction = actions.createNestedObject();
    middleAction["caption"] = "Set louver middle position";
    middleAction["id"] = MIDDLE;
    JsonObject brightAction = actions.createNestedObject();
    brightAction["caption"] = "Set louver bright position";
    brightAction["id"] = BRIGHT;

    JsonArray config = jsonDoc.createNestedArray("config");
    JsonObject config_0 = config.createNestedObject();
    config_0["name"] = CLOSE_SETTING;
    config_0["type"] = "number";
    JsonObject config_1 = config.createNestedObject();
    config_1["name"] = OPEN_SETTING;
    config_1["type"] = "number";
    JsonObject config_2 = config.createNestedObject();
    config_2["name"] = BRIGHT_SETTING;
    config_2["type"] = "number";
    JsonObject config_3 = config.createNestedObject();
    config_3["name"] = DELAY_SETTING;
    config_3["type"] = "number";
    JsonObject config_4 = config.createNestedObject();
    config_4["name"] = ACCURACY_SETTING;
    config_4["type"] = "number";
    JsonObject config_5 = config.createNestedObject();
    config_5["name"] = NAME_SETTING;
    config_5["type"] = "text";

    HandlerResult result;
    result.code = 200;
    result.contentType = JSON_CONTENT_TYPE;
    serializeJson(jsonDoc, result.body);
    return result;
}

HandlerResult handleAction(String body, LouverController * controller) {
    HandlerResult result;

    DynamicJsonDocument jsonDoc(64);
    deserializeJson(jsonDoc, body);

    bool actionResult = false;

    if (jsonDoc.containsKey("action")) {
        int action = jsonDoc["action"];
        switch(action) {
            case ENABLE_AUTO_MODE:
                actionResult = controller->enableAutoMode();
                break;
            case DISABLE_AUTO_MODE:
                actionResult = controller->disableAutoMode();
                break;
            case OPEN:
                actionResult =  controller->open();
                break;
            case CLOSE:
                actionResult = controller->close();
                break;
            case MIDDLE:
                actionResult = controller->middle();
                break;
            case BRIGHT:
                actionResult = controller->bright();
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
    return result;
}

const String buildMainPage(bool apMode) {
    String page = PAGE_PART_1;
    if (apMode) {
        page += SETUP_BLOCK;
    }
    page += PAGE_PART_2;
    return page;
}

#endif