#include "net/rest/RestController.h"
#include "net/rest/Pages.h"
#include "net/rest/handlers/ConfigRequestHandler.h"
#include "net/rest/handlers/WiFiRequestHandler.h"
#include "net/rest/handlers/InfoRequestHandler.h"
#include "net/rest/handlers/SensorsRequestHandler.h"
#include "net/rest/handlers/StateRequestHandler.h"
#include "net/rest/handlers/ActionRequestHandler.h"
#include "net/rest/handlers/CallbacksRequestHandler.h"

#define WEB_SERVER_TAG "web_server"

RestController::RestController(){};
RestController::~RestController(){};

void RestController::begin() {
    setupHandler();
    _server.begin(SERVER_PORT);
    _setupFinished = true;
}

void RestController::reload() {
    if (_setupFinished) {
        _setupFinished = false;
        _server.stop();
    }
    begin();
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
    _server.addHandler(new ActionRequestHandler());
    _server.addHandler(new CallbacksRequestHandler());

    _server.on("/health", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(418, "text/html", "I am alive!!! :)");
    });

    _server.on("/", HTTP_GET, [this]() {
        preHandleRequest();
        _server.send(200, "text/html", WEB_PAGE_MAIN);
    });

    _server.on("/metrics", HTTP_GET, [this]() {
        preHandleRequest();
        DynamicJsonDocument doc(4096);
        doc["uptime"] = millis();

        JsonObject obj = doc.createNestedObject("heap");
        obj["free"] = ESP.getFreeHeap();
        obj["size"] = ESP.getHeapSize();
        obj["minFree"] = ESP.getMinFreeHeap();
        obj["maxAlloc"] = ESP.getMaxAllocHeap();
        obj["settingsUsage"] = STSettings.usage();

        JsonObject counts = doc.createNestedObject("counts");
        counts["sensors"] = SmartThing.getSensorsCount();
        counts["states"] = SmartThing.getDeviceStatesCount();
        counts["callbacks"] = SmartThing.getCallbacksManager()->getTotalCallbacksCount();
        
        String response;
        serializeJson(doc, response);

        _server.send(200, JSON_CONTENT_TYPE, response);
    });
    
    _server.on("/restart", HTTP_PUT, [&](){
        preHandleRequest();

        _server.send(200);
        LOGGER.info(WEB_SERVER_TAG, "---------RESTART---------");
        _restartHandler();

        delay(2000);
        ESP.restart();
    });
    
    _server.onNotFound([&](){
        _server.send(404, "text/plain", "Page not found");
    });
}