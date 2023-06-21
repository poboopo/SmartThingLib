#ifndef OPTIONS_RH_H
#define OPTIONS_RH_H

#include <WebServer.h>

class OptionsRequestHandler: public RequestHandler {
    bool canHandle(HTTPMethod method, String uri) {
        if (method == HTTP_OPTIONS) {
            return true;
        }
        return false;
    }

    bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        // todo
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(200);
        return true; 
    }
};

#endif