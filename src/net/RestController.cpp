#include <net/RestController.h>

#define WEB_SERVER_TAG "web_server"

RestController::RestController(){};
RestController::~RestController(){};

void RestController::begin(SettingsManager * manager) {
    _settingsManager = manager;

    _server.begin(SERVER_PORT);

    setupEndpoints();
    _setupFinished = true;
}

void RestController::handle() {
    if (_setupFinished) {
        _server.handleClient();
    }
}

void RestController::processHandlerResult(HandlerResult result) {
    BetterLogger::log(WEB_SERVER_TAG, "Response code = %d", result.code);
    _server.send(result.code, result.contentType, result.body);
}

String RestController::getRequestBody() {
    if(!_server.hasArg("plain")) {
        return "";
    }
    return _server.arg("plain");
}

String RestController::buildErrorJson(String error) {
    return "{\"error\": \"" + error + "\"}";
}

void RestController::handleConfigPost() {
    String data = getRequestBody();
    if (data.length() == 0) {
        _server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Body is missing"));
        return;
    }

    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, data);
    JsonObject root = jsonDoc.as<JsonObject>();

    for (JsonPair pair: root) {
        _settingsManager->putSetting(GROUP_CONFIG, pair.key().c_str(), pair.value());
    }

    // settingsManager->putSetting(GROUP_CONFIG, jsonDoc.as<JsonObject>());
    _settingsManager->saveSettings();
    _server.send(200);
}

void RestController::handleConfigDelete() {
    if (!_server.hasArg("name")) {
        _server.send(400, "content/json", buildErrorJson("Setting name is missing"));
    }
    String name = _server.arg("name");

    JsonObject config = _settingsManager->getSettings(GROUP_CONFIG);
    if (config.containsKey(name)) {
        BetterLogger::log(WEB_SERVER_TAG, "Removing config value %s", name);
        config.remove(name);
        _settingsManager->saveSettings();
    } else {
        BetterLogger::log(WEB_SERVER_TAG, "Failed to remove config %s - no such key", name);
    }
    _server.send(200);
}

void RestController::setupEndpoints() {
    _server.on("/", HTTP_GET, [&]() {
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/]");
        processHandlerResult(_webPageBuilder());
    });

    _server.on("/info", HTTP_GET, [&]() {
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/info]");
        processHandlerResult(_getInfoHandler());
    });

    _server.on("/health", HTTP_GET, [&]() {
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/health]");
        _server.send(200, "text/html", "I am alive!!! :)");
    });

    _server.on("/dictionary", HTTP_GET, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/dictionary]");
        processHandlerResult(_getDictsHandler());
    });
    _server.on("/state", HTTP_GET, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/state]");
        processHandlerResult(_getStateHandler());
    });
    _server.on("/action", HTTP_PUT, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[PUT] [/state] %s", getRequestBody().c_str());
        processHandlerResult(_actionHandler());
    });
    _server.on("/sensors", HTTP_GET, [&]() {
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/sensors]");
        processHandlerResult(_getSensorsHandler());
    });
    _server.on("/config", HTTP_GET, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[GET] [/config]");
        _server.send(200, "application/json", _settingsManager->getJson(GROUP_CONFIG));
    });
    _server.on("/config", HTTP_POST, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[POST] [/config] %s", getRequestBody().c_str());
        handleConfigPost();
        _configUpdatedHandler();
    });
    _server.on("/config", HTTP_DELETE, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[DELETE] [/config]");
        handleConfigDelete();
        _configUpdatedHandler();
    });

    _server.on("/restart", HTTP_PUT, [&](){
        BetterLogger::log(WEB_SERVER_TAG, "[PUT] [/restart] %s", getRequestBody().c_str());

        HandlerResult result = _getStateHandler();
        _settingsManager->putSetting(GROUP_STATE, result.body);

        _settingsManager->saveSettings();
        _server.send(200);

        BetterLogger::log(WEB_SERVER_TAG, "---------RESTART---------");

        delay(2000);
        ESP.restart();
    });
    _server.onNotFound([&](){
        _server.send(404, "text/plain", "Page not found");
    });
}

void RestController::handleSetupPost() {
    String data = getRequestBody();
    if (data.length() == 0) {
        _server.send(400, "content/json", buildErrorJson("Body is missing"));
        return;
    }

    DynamicJsonDocument jsonDoc(256);
    deserializeJson(jsonDoc, data);
    _settingsManager->putSetting(GROUP_WIFI, SSID_SETTING, jsonDoc["ssid"].as<String>());
    _settingsManager->putSetting(GROUP_WIFI, PASSWORD_SETTING, jsonDoc["password"].as<String>());
    _settingsManager->saveSettings();
    _server.send(200);
}

void RestController::addSetupEndpoint() {
    _server.on("/setup", HTTP_POST, [&]() {
        BetterLogger::log(WEB_SERVER_TAG, "[POST] [/setup] %s", getRequestBody().c_str());
        handleSetupPost();
    });
}