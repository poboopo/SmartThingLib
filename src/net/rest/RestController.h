#ifndef REST_CONTROLLER_H
#define REST_CONTROLLER_H

#include <WebServer.h>
#include "logs/BetterLogger.h"
#include "settings/SettingsManager.h"

#define SERVER_PORT 80
#define JSON_CONTENT_TYPE "application/json"

struct RestHandlerResult{
    int code = 200;
    String contentType = JSON_CONTENT_TYPE;
    String body = "";
};

typedef std::function<void(void)> RestHandlerFunction;

class RestControllerClass{
    public:
        RestControllerClass();
        ~RestControllerClass();
        void begin();
        void reload();
        
        void addConfigUpdatedHandler(RestHandlerFunction hf) {
            _configUpdatedHandler = hf;
        }
        void addWifiupdatedHandler(RestHandlerFunction hf) {
            _wifiUpdatedHandler = hf;
        }
        void addRestartHandler(RestHandlerFunction hf) {
            _restartHandler = hf;
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

        RestHandlerFunction _configUpdatedHandler = [](){};
        RestHandlerFunction _wifiUpdatedHandler = [](){};
        RestHandlerFunction _restartHandler = [](){};
};

extern RestControllerClass RestController;

#endif