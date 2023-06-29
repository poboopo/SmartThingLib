#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include <ArduinoJson.h>
#include <utils/SettingsManager.h>
#include <net/rest/RestController.h>
#include <LouverController.h>

//TODO MOVE THIS FILE TO /utils

#define CLOSE_SETTING "light_close"
#define OPEN_SETTING "light_open"
#define BRIGHT_SETTING "light_bright"
#define DELAY_SETTING "delay"
#define ACCURACY_SETTING "accuracy"

enum LOUVER_ACTIONS {
    DISABLE_AUTO_MODE,
    ENABLE_AUTO_MODE,
    OPEN,
    CLOSE,
    MIDDLE,
    BRIGHT
};

RestHandlerResult getLouverStateJson(LouverController * controller) {
    DynamicJsonDocument jsonDoc(64);
    jsonDoc["automode"] = controller->isAutoModeEnabled();
    jsonDoc["position"] = controller->getMotorPosition();
    jsonDoc["light"] = controller->getLightValue();

    RestHandlerResult result;
    result.code = 200;
    result.contentType = JSON_CONTENT_TYPE;
    serializeJson(jsonDoc, result.body);
    return result;
}

RestHandlerResult getSensorsJson(LouverController * controller) {
    DynamicJsonDocument jsonDoc(64);
    jsonDoc["light"] = controller->getLightValue();

    RestHandlerResult result;
    result.code = 200;
    result.contentType = JSON_CONTENT_TYPE;
    serializeJson(jsonDoc, result.body);
    return result;
}

RestHandlerResult getDictionaries() {
    DynamicJsonDocument jsonDoc(1024);

    JsonArray actions = jsonDoc.createNestedArray("actions");
    JsonObject disableAuto = actions.createNestedObject();
    disableAuto["caption"] = "Disable automode";
    disableAuto["action"] = DISABLE_AUTO_MODE;
    JsonObject enableAuto = actions.createNestedObject();
    enableAuto["caption"] = "Enable automode";
    enableAuto["action"] = ENABLE_AUTO_MODE;
    JsonObject openAction = actions.createNestedObject();
    openAction["caption"] = "Set louver open position";
    openAction["action"] = OPEN;
    JsonObject closeAction = actions.createNestedObject();
    closeAction["caption"] = "Set louver close position";
    closeAction["action"] = CLOSE;
    JsonObject middleAction = actions.createNestedObject();
    middleAction["caption"] = "Set louver middle position";
    middleAction["action"] = MIDDLE;
    JsonObject brightAction = actions.createNestedObject();
    brightAction["caption"] = "Set louver bright position";
    brightAction["action"] = BRIGHT;

    JsonArray config = jsonDoc.createNestedArray("config");
    JsonObject config_0 = config.createNestedObject();
    config_0["caption"] = "Light close value";
    config_0["name"] = CLOSE_SETTING;
    config_0["type"] = "number";
    JsonObject config_1 = config.createNestedObject();
    config_1["caption"] = "Light open value";
    config_1["name"] = OPEN_SETTING;
    config_1["type"] = "number";
    JsonObject config_2 = config.createNestedObject();
    config_2["caption"] = "Light bright value";
    config_2["name"] = BRIGHT_SETTING;
    config_2["type"] = "number";
    JsonObject config_3 = config.createNestedObject();
    config_3["caption"] = "Automode update delay";
    config_3["name"] = DELAY_SETTING;
    config_3["type"] = "number";
    JsonObject config_4 = config.createNestedObject();
    config_4["caption"] = "Motor accuracy";
    config_4["name"] = ACCURACY_SETTING;
    config_4["type"] = "number";

    RestHandlerResult result;
    result.code = 200;
    result.contentType = JSON_CONTENT_TYPE;
    serializeJson(jsonDoc, result.body);
    return result;
}

RestHandlerResult handleAction(String actionArg, LouverController * controller) {
    RestHandlerResult result;

    bool actionResult = false;

    if (!actionArg.isEmpty()) {
        int action = actionArg.toInt();
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
    result.contentType = "text/html";
    return result;
}

#endif