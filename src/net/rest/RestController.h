#ifndef REST_CONTROLLER_H
#define REST_CONTROLLER_H

#include <WebServer.h>
#include <net/logs/BetterLogger.h>
#include <utils/SettingsManager.h>

#define SERVER_PORT 80

#define JSON_CONTENT_TYPE "application/json"

struct HandlerResult{
    int code = 200;
    String contentType = JSON_CONTENT_TYPE;
    String body = "";
};

class RestController{
    public:
        RestController();
        ~RestController();

        typedef std::function<HandlerResult(void)> HandlerWithResultFunction;
        typedef std::function<void(void)> HandlerFunction;
        void begin(SettingsManager * manager);
        
        void addActionHandler(RestController::HandlerWithResultFunction hf) {
            _actionHandler = hf;
        };
        void addGetStateHandler(RestController::HandlerWithResultFunction hf) {
            _getStateHandler = hf;
        };
        void addGetSensorsHandler(RestController::HandlerWithResultFunction hf) {
            _getSensorsHandler = hf;
        };
        void addGetInfoHandler(RestController::HandlerWithResultFunction hf) {
            _getInfoHandler = hf;
        };
        void addGetDictsHandler(RestController::HandlerWithResultFunction hf) {
            _getDictsHandler = hf;
        };
        void addWebPageBuilder(RestController::HandlerWithResultFunction hf) {
            _webPageBuilder = hf;
        };
        void addConfigUpdatedHandler(RestController::HandlerFunction hf) {
            _configUpdatedHandler = hf;
        }

        String getRequestBody();
        String getRequestArg(String name);
        WebServer * getWebServer() { return &_server; };

        void handle();
    private:
        bool _setupFinished = false;
        WebServer _server;
        SettingsManager * _settingsManager;

        void setupHandler();
        void preHandleRequest();

        void processHandlerResult(HandlerResult result);
        HandlerWithResultFunction _defaultHandler = []() {
            HandlerResult result;
            return result;
        };

        HandlerWithResultFunction _getStateHandler = _defaultHandler;
        HandlerWithResultFunction _actionHandler = _defaultHandler;
        HandlerWithResultFunction _webPageBuilder = _defaultHandler;
        HandlerWithResultFunction _getSensorsHandler = _defaultHandler;
        HandlerWithResultFunction _getInfoHandler = _defaultHandler;
        HandlerWithResultFunction _getDictsHandler = _defaultHandler;

        HandlerFunction _configUpdatedHandler = [](){};

        void handleConfigPost();
        void handleConfigDelete();
        void handleWiFiPost();
        void handleWiFiGet();

        String buildErrorJson(String error);
};

#endif