#ifndef STATE_RQ_H
#define STATE_RQ_H

#include "Features.h"
#if ENABLE_STATES

#include "observable/ObservablesManager.h"
#include "logs/BetterLogger.h"
#include "net/rest/WebPageAssets.h"

#define STATE_RQ_PATH "/state"
static const char * STATE_RQ_TAG = "state_handler";

class StateRequestHandler : public AsyncWebHandler {
 public:
  StateRequestHandler(){};
  virtual ~StateRequestHandler(){};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(STATE_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_PUT ||
           request->method() == HTTP_OPTIONS);
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
    st_log_request(STATE_RQ_TAG, request->methodToString(), request->url().c_str(), "");

    #if ENABLE_WEB_PAGE
    if (request->url().equals("/states/script.js")) {
      return request->beginResponse(200, CONTENT_TYPE_JS, SCRIPT_STATES_TAB);
    }
    #endif

    JsonDocument states = ObservablesManager.getDeviceStatesInfo();
    String response;
    serializeJson(states, response);
    return request->beginResponse(200, CONTENT_TYPE_JSON, response);
  }
};

#endif
#endif