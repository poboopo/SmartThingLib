#ifndef ACTION_RQ_H
#define ACTION_RQ_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/handlers/RequestHandler.h"
#include "actions/ActionsManager.h"

#define ACTION_RQ_PATH "/actions"

const char * const _fieldName = "name";
#if ENABLE_ACTIONS_SCHEDULER
const char * const _fieldDelay = "callDelay";
#endif

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
    if (request->method() == HTTP_GET) {
      if (request->url().equals("/actions/info")) {
        return request->beginResponse(200, CONTENT_TYPE_JSON, ActionsManager.toJson());
      }

      if (request->url().equals("/actions/call")) {
        String action = request->arg(_fieldName);
        if (action.isEmpty()) {
          return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Parameter action is missing!"));
        }

        ActionResult result = ActionsManager.call(action.c_str());
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
    }
    
    #if ENABLE_ACTIONS_SCHEDULER
    if (request->method() == HTTP_PUT && request->url().equals("/actions/schedule")) {
      if (_body.isEmpty()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Request body is missing"));
      }
      JsonDocument doc;
      deserializeJson(doc, _body);
      if (!doc[_fieldName].is<JsonVariant>() || !doc[_fieldDelay].is<JsonVariant>()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Name and callDelay params reuqired in body"));
      }
      
      const char * name = doc[_fieldName];
      unsigned long newDelay = doc[_fieldDelay];
      if (ActionsManager.updateActionSchedule(name, newDelay)) {
        return request->beginResponse(200);
      } else {
        return request->beginResponse(500, CONTENT_TYPE_JSON, buildErrorJson("Action delay update failed. Check logs for more information."));
      }
    }
    #endif
    return nullptr;
  }
};

#endif

#endif