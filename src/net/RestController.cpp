#include <net/RestController.h>

#define WEB_SERVER_TAG "web_server"

RestController::RestController(){};
RestController::~RestController(){};

void RestController::begin(BetterLogger * logger, SettingsManager * manager) {
    _logger = logger;
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
    _logger->log(WEB_SERVER_TAG, "Response code = %d", result.code);
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
    _settingsManager->removeSetting(_server.arg("name"));
    _settingsManager->saveSettings();
    _server.send(200);
}

void RestController::setupEndpoints() {
    _server.on("/", HTTP_GET, [&]() {
        _logger->log(WEB_SERVER_TAG, "[GET] [/]");
        processHandlerResult(_webPageBuilder());
    });

    _server.on("/state", HTTP_GET, [&](){
        _logger->log(WEB_SERVER_TAG, "[GET] [/state]");
        processHandlerResult(_getStateHandler());
        _logger->statistics();
    });
    _server.on("/state", HTTP_PUT, [&](){
        _logger->log(WEB_SERVER_TAG, "[PUT] [/state] %s", getRequestBody().c_str());
        processHandlerResult(_changeStateHandler());
        // TODO save state
        _logger->statistics();
    });

    _server.on("/config", HTTP_GET, [&](){
        _logger->log(WEB_SERVER_TAG, "[GET] [/config]");
        _server.send(200, "application/json", _settingsManager->getJson(GROUP_CONFIG));
        _logger->statistics();
    });
    _server.on("/config", HTTP_POST, [&](){
        _logger->log(WEB_SERVER_TAG, "[POST] [/config] %s", getRequestBody().c_str());
        handleConfigPost();
        _configUpdatedHandler();
        _logger->statistics();
    });
    _server.on("/config", HTTP_DELETE, [&](){
        _logger->log(WEB_SERVER_TAG, "[DELETE] [/config]");
        handleConfigDelete();
        _configUpdatedHandler();
        _logger->statistics();
    });
    _server.on("/restart", HTTP_PUT, [&](){
        _logger->log(WEB_SERVER_TAG, "[PUT] [/restart] %s", getRequestBody().c_str());

        HandlerResult result = _getStateHandler();
        _settingsManager->putSetting(GROUP_STATE, result.body);

        _settingsManager->saveSettings();
        _server.send(200);
        _logger->statistics();

        _logger->log(WEB_SERVER_TAG, "---------RESTART---------");

        delay(2000);
        ESP.restart();
    });
    _server.onNotFound([&](){
        _server.send(404, "text/plain", "Page not found");
        _logger->statistics();
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
        _logger->log(WEB_SERVER_TAG, "[POST] [/setup] %s", getRequestBody().c_str());
        handleSetupPost();
    });
}