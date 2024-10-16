#ifndef ACTION_RQ_H
#define ACTION_RQ_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/handlers/RequestHandler.h"

#define ACTION_RQ_PATH "/action"

using namespace Action;

class ActionRequestHandler : public RequestHandler {
 public:
  ActionRequestHandler(){};
  virtual ~ActionRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(ACTION_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_PUT ||
            request->method() == HTTP_OPTIONS);
  };

  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {

    if (request->method() == HTTP_GET && request->url().equals("/actions/info")) {
      JsonDocument doc = SmartThing.getActionsInfo();
      String response;
      serializeJson(doc, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    if (request->method() == HTTP_PUT) {
      String action = request->arg("action");
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