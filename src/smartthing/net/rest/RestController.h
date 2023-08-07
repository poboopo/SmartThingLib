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

        typedef std::function<void(void)> HandlerFunction;
        void begin();
        void reload();
        
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

        void processRestHandlerResult(RestHandlerResult result);\

        HandlerFunction _configUpdatedHandler = [](){};
        HandlerFunction _wifiUpdatedHandler = [](){};

        void handleConfigPost();
        void handleConfigDelete();
        void handleWiFiPost();
        void handleWiFiGet();
};

#endif