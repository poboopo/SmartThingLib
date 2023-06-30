#ifndef SENSORS_RQ_H
#define SENSORS_RQ_H

#include <WebServer.h>
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/SmartThing.h"

#define SENSORS_RQ_PATH "/sensors"
#define SENSORS_RQ_TAG "sesnsors_handler"

class SensorsRequestHandler: public RequestHandler {
    public:
        SensorsRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(SENSORS_RQ_PATH) && 
                (method == HTTP_GET || HTTP_PUT || HTTP_OPTIONS);
        };

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            LOGGER.logRequest(SENSORS_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), "");
            server.sendHeader("Access-Control-Allow-Origin", "*");

            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            if (requestMethod == HTTP_GET) {
                JsonArray sensors = SmartThing.getSensorsValues();
                String response;
                serializeJson(sensors, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }

            return false;
        };
};

#endif