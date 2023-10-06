#ifndef WIFI_RQ_H
#define WIFI_RQ_H

#include <WebServer.h>
#include "settings/SettingsManager.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/RestController.h"

#define WIFI_LOG_TAG "wifi_handler"
#define WIFI_RQ_PATH "/wifi"

class WiFiRequesthandler: public RequestHandler {
    public:
        WiFiRequesthandler(RestController::HandlerFunction * wifiUpdatedHandler): _wifiUpdatedHandler(wifiUpdatedHandler) {};

        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(WIFI_RQ_PATH) && 
                (method == HTTP_GET || method == HTTP_POST || method == HTTP_OPTIONS);
        }

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Origin", "*");
                server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }

            String body = server.arg("plain");
            LOGGER.logRequest(WIFI_LOG_TAG, http_method_str(requestMethod), requestUri.c_str(), body.c_str());
            
            server.sendHeader("Access-Control-Allow-Origin", "*");
            if (requestMethod == HTTP_GET) {
                //todo hide password smh
                DynamicJsonDocument jsonDoc(1028);
                jsonDoc["settings"] = STSettings.getWiFi();
                JsonObject modes = jsonDoc.createNestedObject("modes");
                modes[String(WIFI_MODE_STA)] = "STA";
                modes[String(WIFI_MODE_AP)] = "AP";

                String response;
                serializeJson(jsonDoc, response);

                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            } 
            if (requestMethod == HTTP_POST) {
                if (body.length() == 0) {
                    server.send(400, "content/json", buildErrorJson("Body is missing"));
                    return true;
                }

                DynamicJsonDocument jsonDoc(256);
                deserializeJson(jsonDoc, body);

                String ssid = jsonDoc["ssid"].as<String>();
                if (ssid.isEmpty()) {
                    server.send(400, "content/json", buildErrorJson("Ssid is missing"));
                    return true;
                }
                if (ssid.length() > 32) {
                    server.send(400, "content/json", buildErrorJson("Ssid is too long (32 symbols max)"));
                    return true;
                }
                String password = jsonDoc["password"].as<String>();
                if (password.length() > 0 && password.length() < 8) {
                    server.send(400, "content/json", buildErrorJson("Password is too short (8 symbols or blank)"));
                }
                int mode = jsonDoc["mode"].as<int>();
                if (mode < 0) {
                    server.send(400, "content/json", buildErrorJson("WiFi mode can't be negative"));
                }
                if (mode == WIFI_MODE_NULL) {
                    mode = WIFI_MODE_STA;
                }

                JsonObject wifiSettings = STSettings.getWiFi();
                wifiSettings[SSID_SETTING] = ssid;
                wifiSettings[PASSWORD_SETTING] = password;
                wifiSettings[WIFI_MODE_SETTING] = mode;
                STSettings.save();
                server.send(200);
                if (_wifiUpdatedHandler != nullptr) {
                    (*_wifiUpdatedHandler)();
                }
                return true;
            }
            return false;
        }
    private:
        RestController::HandlerFunction * _wifiUpdatedHandler;
};

#endif