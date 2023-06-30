#ifndef REST_CONTROLLER_H
#define REST_CONTROLLER_H

#include <WebServer.h>
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/settings/SettingsManager.h"

#define SERVER_PORT 80
#define JSON_CONTENT_TYPE "application/json"

struct RestHandlerResult{
    int code = 200;
    String contentType = JSON_CONTENT_TYPE;
    String body = "";
};

class RestController{
    public:
        RestController();
        ~RestController();

        typedef std::function<RestHandlerResult(void)> HandlerWithResultFunction;
        typedef std::function<void(void)> HandlerFunction;
        void begin();
        
        void addActionHandler(RestController::HandlerWithResultFunction hf) {
            _actionHandler = hf;
        };
        void addGetStateHandler(RestController::HandlerWithResultFunction hf) {
            _getStateHandler = hf;
        };
        void addGetSensorsHandler(RestController::HandlerWithResultFunction hf) {
            _getSensorsHandler = hf;
        };
        void addGetDictsHandler(RestController::HandlerWithResultFunction hf) {
            _getDictsHandler = hf;
        };
        void addConfigUpdatedHandler(RestController::HandlerFunction hf) {
            _configUpdatedHandler = hf;
        }
        void addWifiupdatedHandler(RestController::HandlerFunction hf) {
            _wifiUpdatedHandler = hf;
        }

        String getRequestBody();
        String getRequestArg(String name);
        WebServer * getWebServer() { return &_server; };

        void handle();
    private:
        bool _setupFinished = false;
        WebServer _server;

        void setupHandler();
        void preHandleRequest();

        void processRestHandlerResult(RestHandlerResult result);
        HandlerWithResultFunction _defaultHandler = []() {
            RestHandlerResult result;
            return result;
        };

        HandlerWithResultFunction _getStateHandler = _defaultHandler;
        HandlerWithResultFunction _actionHandler = _defaultHandler;
        HandlerWithResultFunction _getSensorsHandler = _defaultHandler;
        HandlerWithResultFunction _getDictsHandler = _defaultHandler;

        HandlerFunction _configUpdatedHandler = [](){};
        HandlerFunction _wifiUpdatedHandler = [](){};

        void handleConfigPost();
        void handleConfigDelete();
        void handleWiFiPost();
        void handleWiFiGet();
};

#endif