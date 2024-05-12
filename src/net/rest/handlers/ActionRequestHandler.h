#ifndef ACTION_RQ_H
#define ACTION_RQ_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include <WebServer.h>

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"

#define ACTION_RQ_PATH "/action"
#define ACTION_RQ_TAG "action_handler"

using namespace Configurable::Action;

class ActionRequestHandler : public RequestHandler {
 public:
  ActionRequestHandler(){};
  bool canHandle(HTTPMethod method, String uri) {
    return uri.startsWith(ACTION_RQ_PATH) &&
           (method == HTTP_GET || method == HTTP_PUT || method == HTTP_OPTIONS);
  };

  bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
    String action = server.arg("action");
    LOGGER.logRequest(ACTION_RQ_TAG, http_method_str(requestMethod),
                      requestUri.c_str(),
                      action.isEmpty() ? "no_action" : action.c_str());
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (requestMethod == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Methods", "PUT, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(200);
      return true;
    }
    if (requestMethod == HTTP_PUT) {
      if (action.isEmpty()) {
        server.send(400, CONTENT_TYPE_JSON,
                    buildErrorJson("Parameter action is missing!"));
        return true;
      }

      ActionResult result = SmartThing.callAction(action.c_str());
      if (result.successful) {
        server.send(200);
      } else {
        if (result.message != nullptr) {
          server.send(500, CONTENT_TYPE_JSON, buildErrorJson(result.message));
        } else {
          server.send(500);
        }
      }
      return true;
    }

    return false;
  };
};

#endif

#endif