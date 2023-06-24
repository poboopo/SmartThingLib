#ifndef CONFIG_RH_H
#define CONFIG_RH_H

#include <WebServer.h>
#include "utils/SettingsManager.h"
#include "net/logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/RestController.h"

#define CONFIG_PATH "/config"
#define CONFIG_LOG_TAG "config_handler"

class ConfigRequestHandler: public RequestHandler {
    public:
        ConfigRequestHandler(RestController::HandlerFunction * configUpdatedHandler): 
            _configUpdatedHandler(configUpdatedHandler) {};

        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(CONFIG_PATH) && 
                (method == HTTP_GET || method == HTTP_OPTIONS || HTTP_POST || HTTP_DELETE);
        }

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            String body = server.arg("plain");
            BetterLogger::logRequest(CONFIG_LOG_TAG, http_method_str(requestMethod), requestUri.c_str(), body.c_str());

            server.sendHeader("Access-Control-Allow-Origin", "*");
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            } else if (requestMethod == HTTP_GET) {
                server.send(200, "application/json", SettingsManager::getJson(GROUP_CONFIG));
                return true; 
            } else if (requestMethod == HTTP_POST) {
                if (body.length() == 0) {
                    server.send(400, "application/json", buildErrorJson("Body is missing"));
                    return true;
                }

                DynamicJsonDocument jsonDoc(1024);
                deserializeJson(jsonDoc, body);
                JsonObject root = jsonDoc.as<JsonObject>();

                for (JsonPair pair: root) {
                    SettingsManager::putSetting(GROUP_CONFIG, pair.key().c_str(), pair.value());
                }

                // settingsManager->putSetting(GROUP_CONFIG, jsonDoc.as<JsonObject>());
                SettingsManager::saveSettings();
                server.send(200);
                callHandler();
                return true; 
            } else if (requestMethod == HTTP_DELETE) {
                if (!server.hasArg("name")) {
                    server.send(400, "content/json", buildErrorJson("Setting name is missing"));
                }
                String name = server.arg("name");

                JsonObject config = SettingsManager::getSettings(GROUP_CONFIG);
                if (config.containsKey(name)) {
                    BetterLogger::log(CONFIG_LOG_TAG, "Removing config value %s", name);
                    config.remove(name);
                    SettingsManager::saveSettings();
                    server.send(200);
                    callHandler();
                } else {
                    BetterLogger::log(CONFIG_LOG_TAG, "Failed to remove config %s - no such key", name);
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