#ifndef HOOKS_RQ_H
#define HOOKS_RQ_H

#include "Features.h"
#if ENABLE_HOOKS 

#include <ArduinoJson.h>
#include <WebServer.h>

#include "SmartThing.h"
#include "hooks/builders/HooksFactory.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"

#define HOOKS_RQ_PATH "/hooks"
#define HOOKS_RQ_TAG "hooks_handler"

#define HOOK_NAME_ARG "name"
#define HOOK_OBSERVABLE_TYPE "type"
#define HOOK_ID_ARG "id"

// todo cut off first /hooks?
class HooksRequestHandler : public RequestHandler {
 public:
  HooksRequestHandler(){};
  bool canHandle(HTTPMethod method, String uri) {
    return uri.startsWith(HOOKS_RQ_PATH) &&
           (method == HTTP_GET || method == HTTP_PUT || method == HTTP_POST ||
            method == HTTP_DELETE || method == HTTP_OPTIONS);
  };

  bool handle(WebServer &server, HTTPMethod requestMethod, String requestUri) {
    String body = server.arg("plain");
    LOGGER.logRequest(HOOKS_RQ_TAG, http_method_str(requestMethod),
                      requestUri.c_str(), body.c_str());

    if (requestMethod == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods",
                        "GET, POST, PUT, DELETE, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(200);
      return true;
    }

    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (requestMethod == HTTP_GET) {
      if (requestUri.equals("/hooks/templates")) {
        String type = server.arg(HOOK_OBSERVABLE_TYPE);
        if (type.isEmpty()) {
          server.send(400, CONTENT_TYPE_JSON,
                      buildErrorJson("Type parameter are missing!"));
          return true;
        }
        DynamicJsonDocument doc = Hook::HooksFactory::getTemplates(type.c_str());
        String response;
        serializeJson(doc, response);
        server.send(200, CONTENT_TYPE_JSON, response);
        return true;
      }
      if (requestUri.equals("/hooks/by/observable")) {
        String type = server.arg(HOOK_OBSERVABLE_TYPE);
        String name = server.arg(HOOK_NAME_ARG);

        if (type.isEmpty() || name.isEmpty()) {
          server.send(
              400, CONTENT_TYPE_JSON,
              buildErrorJson("Observable type or name args are missing!"));
          return true;
        }
        DynamicJsonDocument doc = HooksManager.getObservableHooksJson(
            type.c_str(), name.c_str());
        String response;
        serializeJson(doc, response);
        server.send(200, CONTENT_TYPE_JSON, response);
        return true;
      }
      if (requestUri.equals("/hooks/by/id")) {
        String type = server.arg(HOOK_OBSERVABLE_TYPE);
        String name = server.arg(HOOK_NAME_ARG);
        String id = server.arg(HOOK_ID_ARG);

        if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
          server.send(400, CONTENT_TYPE_JSON,
                      buildErrorJson("Type, name or id args are missing!"));
          return true;
        }
        DynamicJsonDocument doc = HooksManager.getHookJsonById(
            type.c_str(), name.c_str(), id.toInt());
        String response;
        serializeJson(doc, response);
        server.send(200, CONTENT_TYPE_JSON, response);
        return true;
      }
      if (requestUri.equals("/hooks/test")) {
        String type = server.arg(HOOK_OBSERVABLE_TYPE);
        String name = server.arg(HOOK_NAME_ARG);
        String id = server.arg(HOOK_ID_ARG);
        String value = server.arg("value");

        if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
          server.send(400, CONTENT_TYPE_JSON,
                      buildErrorJson("Type, name or id args are missing!"));
          return true;
        }
        if (HooksManager.callHook(type.c_str(), name.c_str(), id.toInt(), value)) {
          server.send(200);
        } else {
          server.send(500);
        }
        return true;
      }
      DynamicJsonDocument doc = HooksManager.allHooksToJson(false, false);
      String response;
      serializeJson(doc, response);
      server.send(200, CONTENT_TYPE_JSON, response);
      return true;
    }
    if (requestMethod == HTTP_POST) {
      if (!server.hasArg("plain")) {
        server.send(400, CONTENT_TYPE_JSON, "Body is missing!");
        return true;
      }
      int id =
          HooksManager.createHookFromJson(server.arg("plain").c_str());
      if (id >= 0) {
        HooksManager.saveHooksToSettings();
        // spritf fails, why?
        DynamicJsonDocument doc(16);
        doc["id"] = id;
        String response;
        serializeJson(doc, response);
        server.send(201, CONTENT_TYPE_JSON, response);
      } else {
        server.send(500, CONTENT_TYPE_JSON,
                    buildErrorJson("Failed to create hook. Check logs for "
                                   "additional information."));
      }
      return true;
    }
    if (requestMethod == HTTP_PUT) {
      String body = server.arg("plain");
      if (body.isEmpty()) {
        server.send(400, CONTENT_TYPE_JSON, buildErrorJson("Body is missing!"));
        return true;
      }

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, body);
      if (HooksManager.updateHook(doc)) {
        HooksManager.saveHooksToSettings();
        server.send(200);
      } else {
        server.send(500, CONTENT_TYPE_JSON,
                    buildErrorJson("Failed to update hook. Check logs for "
                                   "additional information."));
      }
      return true;
    }
    if (requestMethod == HTTP_DELETE) {
      String type = server.arg(HOOK_OBSERVABLE_TYPE);
      String name = server.arg(HOOK_NAME_ARG);
      String id = server.arg(HOOK_ID_ARG);

      if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
        server.send(
            400, CONTENT_TYPE_JSON,
            buildErrorJson("Observable type, name or id args are missing!"));
        return true;
      }

      if (HooksManager.deleteHook(type.c_str(), name.c_str(),
                                          id.toInt())) {
        HooksManager.saveHooksToSettings();
        server.send(200);
      } else {
        server.send(500, CONTENT_TYPE_JSON,
                    buildErrorJson("Failed to delete hook. Check logs for "
                                   "additional information."));
      }
      return true;
    }

    return false;
  };
};
#endif

#endif