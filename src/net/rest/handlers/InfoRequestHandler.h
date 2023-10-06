#ifndef INFO_RQ_H
#define INFO_RQ_H

#include <WebServer.h>
#include "net/rest/RestController.h"
#include "logs/BetterLogger.h"
#include "settings/SettingsManager.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "SmartThing.h"

#define INFO_RQ_PATH "/info"
#define INFO_RQ_TAG "wifi_handler"

class InfoRequestHandler: public RequestHandler {
    public:
        InfoRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(INFO_RQ_PATH) && 
                (method == HTTP_GET || method == HTTP_PUT || method == HTTP_OPTIONS);
        }
        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            String body = server.arg("plain");
            LOGGER.logRequest(INFO_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), body.c_str());
            server.sendHeader("Access-Control-Allow-Origin", "*");
            
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            // todo remove requestUri.equals("/info")
            if (requestUri.equals("/info/system") || requestUri.equals("/info")) {
                if (requestMethod == HTTP_GET) {
                    DynamicJsonDocument jsonDoc(256);
                    jsonDoc["version"] = SMART_THING_VERSION;
                    jsonDoc["name"] = SmartThing.getName();
                    jsonDoc["type"] = SmartThing.getType();
                    jsonDoc["chip_model"] = ESP.getChipModel();
                    jsonDoc["chip_revision"] = ESP.getChipRevision();

                    String result;
                    serializeJson(jsonDoc, result);
                    server.send(200, JSON_CONTENT_TYPE, result);
                    return true;
                }
                if (requestMethod == HTTP_PUT) {
                    DynamicJsonDocument jsDoc(64);
                    deserializeJson(jsDoc, body);
                    const char * newName = jsDoc["name"];
                    if (strlen(newName) == 0 || strlen(newName) > DEVICE_NAME_LENGTH_MAX) {
                        server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Name is missing or too long (max 10 symbols)"));
                        return true;
                    }

                    SmartThing.setName(newName);
                    server.send(200);
                    return true;
                }
            }
            if (requestUri.equals("/info/actions") && requestMethod == HTTP_GET) {
                DynamicJsonDocument doc = SmartThing.getActionsInfo();
                String response;
                serializeJson(doc, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }
            if (requestUri.equals("/info/config") && requestMethod == HTTP_GET) {
                DynamicJsonDocument doc = SmartThing.getConfigInfo();
                String response;
                serializeJson(doc, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }

            return false;
        }
};

#endif