#ifndef SENSORS_RQ_H
#define SENSORS_RQ_H

#include "Features.h"
#if ENABLE_SENSORS 

#include "SmartThing.h"
#include "logs/BetterLogger.h"

#define SENSORS_RQ_PATH "/sensors"
#define SENSORS_RQ_TAG "sensors_handler"

class SensorsRequestHandler : public AsyncWebHandler {
 public:
  SensorsRequestHandler(){};
  virtual ~SensorsRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().equals(SENSORS_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_OPTIONS);
  };

  void handleRequest(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type");
      request->send(response);
      return;
    }

    AsyncWebServerResponse * asyncResponse = processRequest(request);
    if (asyncResponse != nullptr) {
      asyncResponse->addHeader("Access-Control-Allow-Origin", "*");
      request->send(asyncResponse);
    }
  };
 private:
  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    LOGGER.logRequest(SENSORS_RQ_TAG, request->methodToString(), request->url().c_str(), "");

    JsonDocument sensors = SmartThing.getSensorsValues();
    String response;
    serializeJson(sensors, response);
    return request->beginResponse(200, CONTENT_TYPE_JSON, response);
  }
};

#endif
#endif