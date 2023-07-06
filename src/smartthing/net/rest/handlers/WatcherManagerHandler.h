#ifndef WATCHER_RQ_H
#define WATCHER_RQ_H

#include <WebServer.h>
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/SmartThing.h"

#define WATCHER_RQ_PATH "/watchers"
#define WATCHER_RQ_TAG "watchers_handler"

class WatchersRequestHandler: public RequestHandler {
    public:
        WatchersRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(WATCHER_RQ_PATH) && 
                (method == HTTP_GET || HTTP_PUT || HTTP_OPTIONS);
        };

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            LOGGER.logRequest(WATCHER_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), "");
            server.sendHeader("Access-Control-Allow-Origin", "*");

            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            if (requestMethod == HTTP_GET) {
                DynamicJsonDocument doc = SmartThing.getWatchersInfo();
                String response;
                serializeJson(doc, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }

            return false;
        };
};

#endif