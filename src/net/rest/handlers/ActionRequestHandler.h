#ifndef ACTION_RQ_H
#define ACTION_RQ_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"

#define ACTION_RQ_PATH "/action"
#define ACTION_RQ_TAG "action_handler"

using namespace Action;

class ActionRequestHandler : public AsyncWebHandler {
 public:
  ActionRequestHandler(){};
  virtual ~ActionRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(ACTION_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_PUT ||
            request->method() == HTTP_OPTIONS);
  };

  void handleRequest(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
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
    String action = request->arg("action");
    LOGGER.logRequest(ACTION_RQ_TAG, request->methodToString(), request->url().c_str(), action.isEmpty() ? "no_action" : action.c_str());

    if (request->method() == HTTP_PUT) {
      if (action.isEmpty()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Parameter action is missing!"));
      }

      ActionResult result = SmartThing.callAction(action.c_str());
      if (result.successful) {
        return request->beginResponse(200);
      } else {
        if (result.message != nullptr) {
          return request->beginResponse(500, CONTENT_TYPE_JSON, buildErrorJson(result.message));
        } else {
          return request->beginResponse(500);
        }
      }
    }
    return nullptr;
  }
};

#endif

#endif