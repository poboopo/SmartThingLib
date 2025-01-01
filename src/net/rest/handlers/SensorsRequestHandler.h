#ifndef SENSORS_RQ_H
#define SENSORS_RQ_H

#include "Features.h"

#if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS

#include <ESPAsyncWebServer.h>
#include "sensors/SensorsManager.h"
#include "logs/BetterLogger.h"

#define SENSORS_RQ_PATH "/sensors"
const char * const _SENSORS_RQ_TAG = "sensors-handler";

class SensorsRequestHandler : public AsyncWebHandler {
 public:
  SensorsRequestHandler(){};
  virtual ~SensorsRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(SENSORS_RQ_PATH) &&
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
    if (asyncResponse == nullptr) {
      st_log_error(_REQUEST_HANDLER_TAG, "Response = nullptr! Sending 404 response");
      asyncResponse = request->beginResponse(404);
    }

    asyncResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(asyncResponse);
  };
 private:
  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    st_log_request(_SENSORS_RQ_TAG, request->methodToString(), request->url().c_str(), "");

    if (request->url().equals(SENSORS_RQ_PATH)) {
      JsonDocument data = SensorsManager.getSensorsInfo();
      String response;
      serializeJson(data, response);
      return request->beginResponse(200, "application/json", response);
    }

    return nullptr;
  }
};

#endif
#endif