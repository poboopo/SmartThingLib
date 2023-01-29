#include <WebServer.h>
#include <LouverController.h>
#include <ArduinoJson.h>

#define WEB_SERVER_TAG "web_server"

enum LOUVER_ACTIONS {
    DISABLE_AUTO_MODE,
    ENABLE_AUTO_MODE,
    OPEN,
    CLOSE,
    MIDDLE,
    BRIGHT
};

StaticJsonDocument<250> jsonDocument;
char buffer[250];

const char * greetingPage = "Hi there!";

String buildErrorJson(String error) {
    return "{\"error\":\"" + error + "\"}";
}


void handleLouverGet(WebServer * server, LouverController * controller) {
    jsonDocument.clear();
    jsonDocument["automode"] = controller->isAutoModeEnabled();
    jsonDocument["position"] = controller->getMotorPosition();
    jsonDocument["light"] = controller->getLightValue();
    serializeJson(jsonDocument, buffer);
    server->send(200, "application/json", buffer);
}

void handleLouverPut(WebServer * server, LouverController * controller) {
    if (!server->hasArg("plain")) {
        server->send(400);
        return;
    }
    deserializeJson(jsonDocument, server->arg("plain"));
    if (jsonDocument.containsKey("action")) {
        int action = jsonDocument["action"];
        // TODO мне кажется есть элегантное решение, но пока только на это
        // хватило познаний c++
        switch(action) {
            case ENABLE_AUTO_MODE:
                controller->enableAutoMode();
                break;
            case DISABLE_AUTO_MODE:
                controller->disabelAutoMode();
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
        server->send(200);
    }
}