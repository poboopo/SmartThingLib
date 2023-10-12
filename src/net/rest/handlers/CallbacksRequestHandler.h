#ifndef CALLBACKS_RQ_H
#define CALLBACKS_RQ_H

#include <WebServer.h>
#include <ArduinoJson.h>

#include "logs/BetterLogger.h"
#include "SmartThing.h"
#include "net/rest/handlers/HandlerUtils.h"

#define CALLBACKS_RQ_PATH "/callbacks"
#define CALLBACKS_RQ_TAG "callbacks_handler"

#define CALLBACK_TYPE_ARG "type"
#define CALLBACK_NAME_ARG "name"
#define CALLBACK_OBSERVABLE_TYPE "observableType"
#define CALLBACK_ID_ARG "id"

class CallbacksRequestHandler: public RequestHandler {
    public:
        CallbacksRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(CALLBACKS_RQ_PATH) && 
                (method == HTTP_GET || method == HTTP_PUT || method == HTTP_POST || method == HTTP_DELETE || method == HTTP_OPTIONS);
        };

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            String body = server.arg("plain");
            LOGGER.logRequest(CALLBACKS_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), body.c_str());
            
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Origin", "*");
                server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            
            server.sendHeader("Access-Control-Allow-Origin", "*");
            if (requestMethod == HTTP_GET) {
                if (requestUri.equals("/callbacks/template")) {
                    DynamicJsonDocument doc = SmartThing.getCallbacksManager()->getCallbacksTemplates();
                    String response;
                    serializeJson(doc, response);
                    server.send(200, JSON_CONTENT_TYPE, response);
                    return true;
                }
                if (requestUri.equals("/callbacks/by/observable")) {
                    String type = server.arg(CALLBACK_OBSERVABLE_TYPE);
                    String name = server.arg(CALLBACK_NAME_ARG);

                    if (type.isEmpty() || name.isEmpty()) {
                        server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Observable type or name args are missing!"));
                        return true;
                    }
                    DynamicJsonDocument doc = SmartThing.getCallbacksManager()->getCallbacksJson(type.c_str(), name.c_str());
                    String response;
                    serializeJson(doc, response);
                    server.send(200, JSON_CONTENT_TYPE, response);
                    return true;
                }
                DynamicJsonDocument doc = SmartThing.getCallbacksManager()->callbacksToJson(false, false);
                String response;
                serializeJson(doc, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }
            if (requestMethod == HTTP_POST && requestUri.equals("/callbacks/create")) {
                if (!server.hasArg("plain")) {
                    server.send(400, JSON_CONTENT_TYPE, "Body is missin!");
                    return true;
                }
                if (SmartThing.getCallbacksManager()->createCallbackFromJson(server.arg("plain").c_str())) {
                    SmartThing.getCallbacksManager()->saveCallbacksToSettings();
                    server.send(201);
                } else {
                    server.send(500, JSON_CONTENT_TYPE, buildErrorJson("Failed to create watcher. Check logs for additional information."));
                }
                return true;
            }
            if (requestMethod == HTTP_PUT && requestUri.equals("/callbacks/update")) {
                String body = server.arg("plain");
                if (body.isEmpty()) {
                    server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Body is missing!"));
                    return true;
                }
                
                if(SmartThing.getCallbacksManager()->updateCallback(body.c_str())) {
                    SmartThing.getCallbacksManager()->saveCallbacksToSettings();
                    server.send(200);
                } else {
                    server.send(500, JSON_CONTENT_TYPE, buildErrorJson("Failed to update callback. Check logs for additional information."));
                }
                return true;
            }
            if (requestMethod == HTTP_DELETE && requestUri.equals("/callbacks/delete")) {
                String type = server.arg(CALLBACK_OBSERVABLE_TYPE);
                String name = server.arg(CALLBACK_NAME_ARG);
                String id = server.arg(CALLBACK_ID_ARG);

                if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
                    server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Observable type, name or id args are missing!"));
                    return true;
                }
                if(SmartThing.getCallbacksManager()->deleteCallback(type.c_str(), name.c_str(), id.toInt())) {
                    SmartThing.getCallbacksManager()->saveCallbacksToSettings();
                    server.send(200);
                } else {
                    server.send(500, JSON_CONTENT_TYPE, buildErrorJson("Failed to delete callback. Check logs for additional information."));
                }
                return true;
            }

            return false;
        };
};

#endif