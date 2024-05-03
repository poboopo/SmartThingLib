#ifndef SENSORS_RQ_H
#define SENSORS_RQ_H

#include "Features.h"
#if ENABLE_SENSORS 

#include <WebServer.h>

#include "SmartThing.h"
#include "logs/BetterLogger.h"

#define SENSORS_RQ_PATH "/sensors"
#define SENSORS_RQ_TAG "sensors_handler"

class SensorsRequestHandler : public RequestHandler {
 public:
  SensorsRequestHandler(){};
  bool canHandle(HTTPMethod method, String uri) {
    return uri.startsWith(SENSORS_RQ_PATH) &&
           (method == HTTP_GET || method == HTTP_PUT || method == HTTP_OPTIONS);
  };

  bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
    LOGGER.logRequest(SENSORS_RQ_TAG, http_method_str(requestMethod),
                      requestUri.c_str(), "");
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (requestMethod == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(200);
      return true;
    }
    if (requestMethod == HTTP_GET) {
      DynamicJsonDocument sensors = SmartThing.getSensorsValues();
      String response;
      serializeJson(sensors, response);
      server.send(200, CONTENT_TYPE_JSON, response);
      return true;
    }

    return false;
  };
};

#endif
#endif