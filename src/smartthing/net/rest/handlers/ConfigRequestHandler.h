#ifndef CONFIG_RH_H
#define CONFIG_RH_H

#include <WebServer.h>
#include "smartthing/settings/SettingsManager.h"
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/net/rest/handlers/HandlerUtils.h"
#include "smartthing/net/rest/RestController.h"

#define CONFIG_PATH "/config"
#define CONFIG_LOG_TAG "config_handler"

class ConfigRequestHandler: public RequestHandler {
    public:
        ConfigRequestHandler(RestController::HandlerFunction * configUpdatedHandler): 
            _configUpdatedHandler(configUpdatedHandler) {};

        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(CONFIG_PATH) && 
                (method == HTTP_GET || method == HTTP_OPTIONS || method == HTTP_POST || method == HTTP_DELETE);
        }

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            String body = server.arg("plain");
            LOGGER.logRequest(CONFIG_LOG_TAG, http_method_str(requestMethod), requestUri.c_str(), body.c_str());

            server.sendHeader("Access-Control-Allow-Origin", "*");
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            } else if (requestMethod == HTTP_GET) {
                JsonObject config = STSettings.getConfig();
                String response;
                serializeJson(config, response);
                server.send(200, "application/json", response);
                return true; 
            } else if (requestMethod == HTTP_POST) {
                if (body.length() == 0) {
                    server.send(400, "application/json", buildErrorJson("Body is missing"));
                    return true;
                }

                DynamicJsonDocument jsonDoc(1024);
                deserializeJson(jsonDoc, body);
                JsonObject root = jsonDoc.as<JsonObject>();
                JsonObject config = STSettings.getConfig();

                //  bad realisation
                //  why i made it????
                for (JsonPair pair: root) {
                    config[pair.key().c_str()] = pair.value();
                }

                STSettings.saveSettings();
                server.send(200);
                callHandler();
                return true; 
            } else if (requestMethod == HTTP_DELETE) {
                if (!server.hasArg("name")) {
                    server.send(400, "content/json", buildErrorJson("Setting name is missing"));
                }
                String name = server.arg("name");

                JsonObject config = STSettings.getConfig();
                if (config.containsKey(name)) {
                    LOGGER.warning(CONFIG_LOG_TAG, "Removing config value %s", name);
                    config.remove(name);
                    STSettings.saveSettings();
                    server.send(200);
                    callHandler();
                } else {
                    LOGGER.error(CONFIG_LOG_TAG, "Failed to remove config %s - no such key", name);
                    server.send(404, "content/json", buildErrorJson("No such key"));
                }
                return true;
            }
            return false;
        }
    private:
        RestController::HandlerFunction * _configUpdatedHandler;

        void callHandler() {
            if (_configUpdatedHandler != nullptr) {
                (*_configUpdatedHandler)();
            }
        }
};

#endif