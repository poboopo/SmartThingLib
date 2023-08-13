#ifndef DICT_RQ_H
#define DICT_RQ_H

#include <WebServer.h>
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/SmartThing.h"

#define DICT_RQ_PATH "/dictionary"
#define DICT_RQ_TAG "dictionary_handler"

class DictionaryRequestHandler: public RequestHandler {
    public:
        DictionaryRequestHandler() {};
        bool canHandle(HTTPMethod method, String uri) {
            return uri.startsWith(DICT_RQ_PATH) && 
                (method == HTTP_GET || method == HTTP_PUT || method == HTTP_OPTIONS);
        };

        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
            LOGGER.logRequest(DICT_RQ_TAG, http_method_str(requestMethod), requestUri.c_str(), "");
            server.sendHeader("Access-Control-Allow-Origin", "*");

            if (requestMethod == HTTP_OPTIONS) {
                server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
                server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
                server.send(200);
                return true; 
            }
            if (requestMethod == HTTP_GET) {
                // todo add /actions and /config
                DynamicJsonDocument doc = SmartThing.getInfoDictionaries();
                String response;
                serializeJson(doc, response);
                server.send(200, JSON_CONTENT_TYPE, response);
                return true;
            }

            return false;
        };
};

#endif