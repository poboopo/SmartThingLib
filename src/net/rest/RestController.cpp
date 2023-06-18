#include <net/rest/RestController.h>
#include <net/rest/Pages.h>
#include <net/rest/OptionsRequestHandler.h>

#define WEB_SERVER_TAG "web_server"

RestController::RestController(){};
RestController::~RestController(){};

void RestController::begin(SettingsManager * manager) {
    _settingsManager = manager;

    setupHandler();
    _server.begin(SERVER_PORT);
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

String RestController::getRequestArg(String name) {
    if (!_server.hasArg(name)) {
        return "";
    }
    return _server.arg(name);
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

void RestController::preHandleRequest() {
    BetterLogger::log(
        WEB_SERVER_TAG, 
        "[%s] [%s] %s", 
        http_method_str(_server.method()),
        _server.uri(),
        getRequestBody().c_str()
    );
    _server.sendHeader("Access-Control-Allow-Origin", "*");
}

// add authorization?
// TODO MOVE ALL HANDLERS TO DIFFERENT CLASSES!!!
void RestController::setupHandler() {
    _server.addHandler(new OptionsRequestHandler());

    _server.on("/health", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", "I am alive!!! :)");
    });

    _server.on("/", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", WEB_PAGE_MAIN);
    });

    _server.on("/info", HTTP_GET, [this]() {
        preHandleRequest();
        processHandlerResult(_getInfoHandler());
    });

    _server.on("/dictionary", HTTP_GET, [this](){
        preHandleRequest();
        processHandlerResult(_getDictsHandler());
    });
    _server.on("/state", HTTP_GET, [this](){
        preHandleRequest();
        processHandlerResult(_getStateHandler());
    });
    _server.on("/action", HTTP_PUT, [this](){
        BetterLogger::log(WEB_SERVER_TAG, "[PUT] [/action] %s", getRequestArg("action").c_str());
        _server.sendHeader("Access-Control-Allow-Origin", "*");
        processHandlerResult(_actionHandler());
    });
    _server.on("/sensors", HTTP_GET, [this]() {
        preHandleRequest();
        processHandlerResult(_getSensorsHandler());
    });
    _server.on("/config", HTTP_GET, [this](){
        preHandleRequest();
        _server.send(200, "application/json", _settingsManager->getJson(GROUP_CONFIG));
    });
    _server.on("/config", HTTP_POST, [this](){
        preHandleRequest();
        handleConfigPost();
        _configUpdatedHandler();
    });
    _server.on("/config", HTTP_DELETE, [this](){
        preHandleRequest();
        handleConfigDelete();
        _configUpdatedHandler();
    });

    _server.on("/wifi", HTTP_POST, [this]() {
        preHandleRequest();
        handleWiFiPost();
    });

    _server.on("/wifi", HTTP_GET, [this]() {
        preHandleRequest();
        handleWiFiGet();
    });

    _server.on("/restart", HTTP_PUT, [&](){
        preHandleRequest();

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

void RestController::handleWiFiGet() {
    String ssid = _settingsManager->getSettingString(GROUP_WIFI, SSID_SETTING);
    String password = _settingsManager->getSettingString(GROUP_WIFI, PASSWORD_SETTING);
    _server.send(200, JSON_CONTENT_TYPE,"{\"ssid\": \"" + ssid + "\", \"password\": \"" + (password.length() > 0 ? "********" : "") + "\"}");
}

void RestController::handleWiFiPost() {
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