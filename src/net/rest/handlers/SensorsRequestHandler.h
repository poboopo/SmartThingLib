#ifndef SENSORS_RQ_H
#define SENSORS_RQ_H

#include "Features.h"

#if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS

#include <ESPAsyncWebServer.h>
#include "sensors/SensorsManager.h"
#include "logs/BetterLogger.h"
#include "net/rest/WebPageAssets.h"

#define OBSERVABLES_RQ_PATH "/sensors"
const char * const _OBSERVABLES_RQ_TAG = "sensors-handler";

// todo rename?
class SensorsRequestHandler : public AsyncWebHandler {
 public:
  SensorsRequestHandler(){};
  virtual ~SensorsRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(OBSERVABLES_RQ_PATH) &&
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
    st_log_request(_OBSERVABLES_RQ_TAG, request->methodToString(), request->url().c_str(), "");

    if (request->url().equals(OBSERVABLES_RQ_PATH)) {
      JsonDocument data = SensorsManager.getSensorsInfo();
      String response;
      serializeJson(data, response);
      return request->beginResponse(200, "application/json", response);
    }

    #if ENABLE_WEB_PAGE
    if (request->url().equals("/sensors/script.js")) {
      return request->beginResponse(200, "text/javascript", SCRIPT_SENSORS_TAB);
    }
    #endif

    return nullptr;
  }
};

#endif
#endif