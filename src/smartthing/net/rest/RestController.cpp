#include "smartthing/net/rest/RestController.h"
#include "smartthing/net/rest/Pages.h"
#include "smartthing/net/rest/handlers/ConfigRequestHandler.h"
#include "smartthing/net/rest/handlers/WiFiRequestHandler.h"
#include "smartthing/net/rest/handlers/InfoRequestHandler.h"
#include "smartthing/net/rest/handlers/SensorsRequestHandler.h"
#include "smartthing/net/rest/handlers/StateRequestHandler.h"
#include "smartthing/net/rest/handlers/DictionaryRequestHandler.h"
#include "smartthing/net/rest/handlers/ActionRequestHandler.h"
#include "smartthing/net/rest/handlers/WatcherManagerHandler.h"

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
    _server.addHandler(new SensorsRequestHandler());
    _server.addHandler(new StateRequestHandler());
    _server.addHandler(new DictionaryRequestHandler());
    _server.addHandler(new ActionRequestHandler());
    _server.addHandler(new WatchersRequestHandler());

    _server.on("/health", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", "I am alive!!! :)");
    });

    _server.on("/", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", WEB_PAGE_MAIN);
    });
    
    _server.on("/restart", HTTP_PUT, [&](){
        preHandleRequest();

        // add restart handler
        // RestHandlerResult result = _getStateHandler();
        // STSettings.putSetting(GROUP_STATE, result.body);
        // STSettings.saveSettings();
        _server.send(200);

        LOGGER.info(WEB_SERVER_TAG, "---------RESTART---------");

        delay(2000);
        ESP.restart();
    });

    _server.onNotFound([&](){
        _server.send(404, "text/plain", "Page not found");
    });
}