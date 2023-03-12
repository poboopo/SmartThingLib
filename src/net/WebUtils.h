#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include <net/Pages.h>
#include <ArduinoJson.h>
#include <utils/SettingsManager.h>
#include <net/RestController.h>
#include <LouverController.h>

//TODO MOVE THIS FILE TO /utils

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

HandlerResult changeLouverState(String body, LouverController * controller) {
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