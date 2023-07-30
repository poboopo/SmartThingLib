#ifndef WATCHER_RQ_H
#define WATCHER_RQ_H

#include <WebServer.h>
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/SmartThing.h"

#define WATCHER_RQ_PATH "/watchers"
#define WATCHER_RQ_TAG "watchers_handler"

#define WATCHER_CALLBACKS_PATH "/watchers/callbacks"

#define WATCHER_TYPE_ARG "type"
#define WATCHER_NAME_ARG "name"

class WatchersRequestHandler: public RequestHandler {
    public:
        WatchersRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(WATCHER_RQ_PATH) && 
                (method == HTTP_GET || HTTP_PUT || HTTP_OPTIONS);
        };

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Origin", "*");
                server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            
            LOGGER.logRequest(WATCHER_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), "");
            server.sendHeader("Access-Control-Allow-Origin", "*");

            if (requestMethod == HTTP_GET) {
                if (strcmp(requestUri.c_str(), WATCHER_CALLBACKS_PATH) == 0) {
                    String type = server.arg(WATCHER_TYPE_ARG);
                    String name = server.arg(WATCHER_NAME_ARG);

                    if (type.isEmpty() || name.isEmpty()) {
                        server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Watcher type or name args are missing!"));
                        return true;
                    }
                    DynamicJsonDocument doc = SmartThing.getCallbacksInfo(type.c_str(), name.c_str());
                    String response;
                    serializeJson(doc, response);
                    server.send(200, JSON_CONTENT_TYPE, response);
                    return true;
                }
                
                DynamicJsonDocument doc = SmartThing.getWatchersInfo();
                String response;
                serializeJson(doc, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }
            if (requestMethod == HTTP_POST) {
                if (!server.hasArg("plain")) {
                    server.send(400, JSON_CONTENT_TYPE, "Body is missin!");
                    return true;
                }
                if (SmartThing.createWatcher(server.arg("plain").c_str())) {
                    server.send(201);
                } else {
                    server.send(500, JSON_CONTENT_TYPE, buildErrorJson("Failed to create watcher. Check logs for additional information."));
                }
                return true;
            }

            return false;
        };
};

#endif