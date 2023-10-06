#ifndef STATE_RQ_H
#define STATE_RQ_H

#include <WebServer.h>
#include "logs/BetterLogger.h"
#include "SmartThing.h"

#define STATE_RQ_PATH "/state"
#define STATE_RQ_TAG "state_handler"

class StateRequestHandler: public RequestHandler {
    public:
        StateRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(STATE_RQ_PATH) && 
                (method == HTTP_GET || method == HTTP_PUT || method == HTTP_OPTIONS);
        };

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            LOGGER.logRequest(STATE_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), "");
            server.sendHeader("Access-Control-Allow-Origin", "*");

            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            if (requestMethod == HTTP_GET) {
                DynamicJsonDocument state = SmartThing.getDeviceStatesInfo();
                String response;
                serializeJson(state, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }

            return false;
        };
};

#endif