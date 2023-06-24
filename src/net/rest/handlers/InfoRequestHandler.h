#ifndef INFO_RQ_H
#define INFO_RQ_H

#include <WebServer.h>
#include "net/rest/RestController.h"
#include "net/logs/BetterLogger.h"
#include "utils/SettingsManager.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "SmartThing.h"

#define INFO_RQ_PATH "/info"
#define INFO_RQ_TAG "wifi_handler"

class InfoRequestHandler: public RequestHandler {
    public:
        InfoRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(INFO_RQ_PATH) && 
                (method == HTTP_GET || HTTP_PUT || HTTP_OPTIONS);
        }
        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Origin", "*");
                server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }

            String body = server.arg("plain");
            BetterLogger::logRequest(INFO_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), body.c_str());
            
            if (requestMethod == HTTP_GET) {
                DynamicJsonDocument jsonDoc(256);
                jsonDoc["version"] = SMART_THING_VERSION;
                jsonDoc["name"] = SmartThing::getName();
                jsonDoc["type"] = SmartThing::getType();
                jsonDoc["chip_model"] = ESP.getChipModel();
                jsonDoc["chip_revision"] = ESP.getChipRevision();

                String result;
                serializeJson(jsonDoc, result);
                server.send(200, JSON_CONTENT_TYPE, result);
                return true;
            } else if (requestMethod == HTTP_PUT) {
                DynamicJsonDocument jsDoc(64);
                deserializeJson(jsDoc, body);
                const char * newName = jsDoc["name"];
                if (strlen(newName) == 0 || strlen(newName) > DEVICE_NAME_LENGTH_MAX) {
                    server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Name is missing or too long (max 10 symbols)"));
                    return true;
                }

                BetterLogger::log(INFO_RQ_TAG, "Got new name %s", newName);
                SmartThing::setName(newName);
                server.send(200);
                return true;
            }

            return false;
        }
};

#endif