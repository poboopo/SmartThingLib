#include <net/Pages.h>
#include <ArduinoJson.h>
#include <utils/SettingsManager.h>

#define WEB_SERVER_TAG "web_server"

enum LOUVER_ACTIONS {
    DISABLE_AUTO_MODE,
    ENABLE_AUTO_MODE,
    OPEN,
    CLOSE,
    MIDDLE,
    BRIGHT
};

String buildErrorJson(String error) {
    return "{\"error\":\"" + error + "\"}";
}

void handleLouverGet(WebServer * server, LouverController * controller) {
    DynamicJsonDocument jsonDoc(64);
    jsonDoc["automode"] = controller->isAutoModeEnabled();
    jsonDoc["position"] = controller->getMotorPosition();
    jsonDoc["light"] = controller->getLightValue();

    String json;
    serializeJson(jsonDoc, json);

    server->send(200, "application/json", json);
}

void handleLouverPut(WebServer * server, LouverController * controller) {
    if (!server->hasArg("plain")) {
        server->send(400);
        return;
    }
    DynamicJsonDocument jsonDoc(64);
    deserializeJson(jsonDoc, server->arg("plain"));

    if (jsonDoc.containsKey("action")) {
        int action = jsonDoc["action"];
        // TODO мне кажется есть элегантное решение, но пока только на это
        // хватило познаний c++
        switch(action) {
            case ENABLE_AUTO_MODE:
                controller->enableAutoMode();
                break;
            case DISABLE_AUTO_MODE:
                controller->disableAutoMode();
                break;
            case OPEN:
                controller->open();
                break;
            case CLOSE:
                controller->close();
                break;
            case MIDDLE:
                controller->middle();
                break;
            case BRIGHT:
                controller->bright();
                break;
            default:
                server->send(400, "application/json", buildErrorJson(String("Wrong action ") + action));
                return;
        }
    }
    
    server->send(200);
}

void handleSettingsPost(WebServer * server, SettingsManager * settingsManager) {
    if (!server->hasArg("plain")) {
        server->send(400, "content/json", buildErrorJson("Body is missing"));
        return;
    }
    String data = server->arg("plain");
    if (data.length() == 0) {
        server->send(400, "content/json", buildErrorJson("Body is missing"));
        return;
    }

    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, data);
    JsonObject root = jsonDoc.as<JsonObject>();

    for (JsonPair pair: root) {
        settingsManager->putSetting(GROUP_CONFIG, pair.key().c_str(), pair.value());
    }

    // settingsManager->putSetting(GROUP_CONFIG, jsonDoc.as<JsonObject>());
    settingsManager->saveSettings();

    server->send(200);
}

void handleSetup(WebServer * server, SettingsManager * settingsManager) {
    if (!server->hasArg("plain")) {
        server->send(400, "content/json", buildErrorJson("Body is missing"));
        return;
    }
    String data = server->arg("plain");
    if (data.length() == 0) {
        server->send(400, "content/json", buildErrorJson("Body is missing"));
        return;
    }

    DynamicJsonDocument jsonDoc(256);
    deserializeJson(jsonDoc, data);
    settingsManager->putSetting(GROUP_WIFI, SSID_SETTING, jsonDoc["ssid"].as<String>());
    settingsManager->putSetting(GROUP_WIFI, PASSWORD_SETTING, jsonDoc["password"].as<String>());
    settingsManager->saveSettings();

    server->send(200);
}

const String buildMainPage(bool apMode) {
    String page = PAGE_PART_1;
    if (apMode) {
        page += SETUP_BLOCK;
    }
    page += PAGE_PART_2;
    return page;
}