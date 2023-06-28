#include <net/rest/RestController.h>
#include <net/rest/Pages.h>
#include <net/rest/handlers/ConfigRequestHandler.h>
#include <net/rest/handlers/WiFiRequestHandler.h>
#include <net/rest/handlers/InfoRequestHandler.h>

#define WEB_SERVER_TAG "web_server"

RestController::RestController(){};
RestController::~RestController(){};

void RestController::begin() {
    setupHandler();
    _server.begin(SERVER_PORT);
    _setupFinished = true;
}

void RestController::handle() {
    if (_setupFinished) {
        _server.handleClient();
    }
}

void RestController::processRestHandlerResult(RestHandlerResult result) {
    LOGGER.info(WEB_SERVER_TAG, "Response code = %d", result.code);
    _server.send(result.code, result.contentType, result.body);
}

String RestController::getRequestBody() {
    return _server.arg("plain");
}

String RestController::getRequestArg(String name) {
    return _server.arg(name);
}

void RestController::preHandleRequest() {
    LOGGER.logRequest(
        WEB_SERVER_TAG, 
        http_method_str(_server.method()),
        _server.uri().c_str(),
        getRequestBody().c_str()
    );
    _server.sendHeader("Access-Control-Allow-Origin", "*");
}

// add authorization?
void RestController::setupHandler() {
    _server.addHandler(new ConfigRequestHandler(&_configUpdatedHandler));
    _server.addHandler(new WiFiRequesthandler(&_wifiUpdatedHandler));
    _server.addHandler(new InfoRequestHandler());

    _server.on("/health", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", "I am alive!!! :)");
    });

    _server.on("/", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", WEB_PAGE_MAIN);
    });

    _server.on("/dictionary", HTTP_GET, [this](){
        preHandleRequest();
        processRestHandlerResult(_getDictsHandler());
    });
    _server.on("/state", HTTP_GET, [this](){
        preHandleRequest();
        processRestHandlerResult(_getStateHandler());
    });
    _server.on("/action", HTTP_PUT, [this](){
        LOGGER.info(WEB_SERVER_TAG, "[PUT] [/action] %s", getRequestArg("action").c_str());
        _server.sendHeader("Access-Control-Allow-Origin", "*");
        processRestHandlerResult(_actionHandler());
    });
    _server.on("/sensors", HTTP_GET, [this]() {
        preHandleRequest();
        processRestHandlerResult(_getSensorsHandler());
    });

    _server.on("/restart", HTTP_PUT, [&](){
        preHandleRequest();

        RestHandlerResult result = _getStateHandler();
        SettingsManager::putSetting(GROUP_STATE, result.body);

        SettingsManager::saveSettings();
        _server.send(200);

        LOGGER.info(WEB_SERVER_TAG, "---------RESTART---------");

        delay(2000);
        ESP.restart();
    });

    _server.onNotFound([&](){
        _server.send(404, "text/plain", "Page not found");
    });
}