#ifndef SENSORS_RQ_H
#define SENSORS_RQ_H

#include "Features.h"
#if ENABLE_SENSORS 

#include "observable/ObservablesManager.h"
#include "logs/BetterLogger.h"
#include "net/rest/WebPageAssets.h"

#define SENSORS_RQ_PATH "/sensors"
static const char * _SENSORS_RQ_TAG = "sensors_handler";

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
    if (asyncResponse != nullptr) {
      asyncResponse->addHeader("Access-Control-Allow-Origin", "*");
      request->send(asyncResponse);
    }
  };
 private:
  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    String url = request->url();
    st_log_request(_SENSORS_RQ_TAG, request->methodToString(), url.c_str(), "");

    #if ENABLE_WEB_PAGE
    if (url.equals("/sensors/script.js")) {
      return request->beginResponse(200, CONTENT_TYPE_JS, SCRIPT_SENSORS_TAB);
    }
    #endif

    JsonDocument data = url.equals("/sensors/types") ? ObservablesManager.getSensorsTypes() : ObservablesManager.getSensorsValues();
    String response;
    serializeJson(data, response);
    return request->beginResponse(200, CONTENT_TYPE_JSON, response);
  }
};

#endif
#endif