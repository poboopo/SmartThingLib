#ifndef CALLBACKS_RQ_H
#define CALLBACKS_RQ_H

#include <ArduinoJson.h>
#include <WebServer.h>

#include "SmartThing.h"
#include "callbacks/builders/CallbacksFactory.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"

#define CALLBACKS_RQ_PATH "/callback"
#define CALLBACKS_RQ_TAG "callbacks_handler"

#define CALLBACK_NAME_ARG "name"
#define CALLBACK_OBSERVABLE_TYPE "type"
#define CALLBACK_ID_ARG "id"

class CallbacksRequestHandler : public RequestHandler {
 public:
  CallbacksRequestHandler(){};
  bool canHandle(HTTPMethod method, String uri) {
    return uri.startsWith(CALLBACKS_RQ_PATH) &&
           (method == HTTP_GET || method == HTTP_PUT || method == HTTP_POST ||
            method == HTTP_DELETE || method == HTTP_OPTIONS);
  };

  bool handle(WebServer &server, HTTPMethod requestMethod, String requestUri) {
    String body = server.arg("plain");
    LOGGER.logRequest(CALLBACKS_RQ_TAG, http_method_str(requestMethod),
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
      if (requestUri.equals("/callback/template")) {
        DynamicJsonDocument doc = Callback::CallbacksFactory::getTemplates();
        String response;
        serializeJson(doc, response);
        server.send(200, JSON_CONTENT_TYPE, response);
        return true;
      }
      if (requestUri.equals("/callback/by/observable")) {
        String type = server.arg(CALLBACK_OBSERVABLE_TYPE);
        String name = server.arg(CALLBACK_NAME_ARG);

        if (type.isEmpty() || name.isEmpty()) {
          server.send(
              400, JSON_CONTENT_TYPE,
              buildErrorJson("Observable type or name args are missing!"));
          return true;
        }
        DynamicJsonDocument doc = CallbacksManager.getObservableCallbacksJson(
            type.c_str(), name.c_str());
        String response;
        serializeJson(doc, response);
        server.send(200, JSON_CONTENT_TYPE, response);
        return true;
      }
      if (requestUri.equals("/callback/by/id")) {
        String type = server.arg(CALLBACK_OBSERVABLE_TYPE);
        String name = server.arg(CALLBACK_NAME_ARG);
        String id = server.arg(CALLBACK_ID_ARG);

        if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
          server.send(400, JSON_CONTENT_TYPE,
                      buildErrorJson("Type, name or id args are missing!"));
          return true;
        }
        DynamicJsonDocument doc = CallbacksManager.getCallbackJsonById(
            type.c_str(), name.c_str(), id.toInt());
        String response;
        serializeJson(doc, response);
        server.send(200, JSON_CONTENT_TYPE, response);
        return true;
      }
      DynamicJsonDocument doc =
          CallbacksManager.allCallbacksToJson(false, false);
      String response;
      serializeJson(doc, response);
      server.send(200, JSON_CONTENT_TYPE, response);
      return true;
    }
    if (requestMethod == HTTP_POST) {
      if (!server.hasArg("plain")) {
        server.send(400, JSON_CONTENT_TYPE, "Body is missing!");
        return true;
      }
      int id =
          CallbacksManager.createCallbackFromJson(server.arg("plain").c_str());
      if (id >= 0) {
        CallbacksManager.saveCallbacksToSettings();
        // spritf fails, why?
        DynamicJsonDocument doc(16);
        doc["id"] = id;
        String response;
        serializeJson(doc, response);
        server.send(201, JSON_CONTENT_TYPE, response);
      } else {
        server.send(500, JSON_CONTENT_TYPE,
                    buildErrorJson("Failed to create callback. Check logs for "
                                   "additional information."));
      }
      return true;
    }
    if (requestMethod == HTTP_PUT) {
      String body = server.arg("plain");
      if (body.isEmpty()) {
        server.send(400, JSON_CONTENT_TYPE, buildErrorJson("Body is missing!"));
        return true;
      }

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, body);
      if (CallbacksManager.updateCallback(doc)) {
        CallbacksManager.saveCallbacksToSettings();
        server.send(200);
      } else {
        server.send(500, JSON_CONTENT_TYPE,
                    buildErrorJson("Failed to update callback. Check logs for "
                                   "additional information."));
      }
      return true;
    }
    if (requestMethod == HTTP_DELETE) {
      String type = server.arg(CALLBACK_OBSERVABLE_TYPE);
      String name = server.arg(CALLBACK_NAME_ARG);
      String id = server.arg(CALLBACK_ID_ARG);

      if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
        server.send(
            400, JSON_CONTENT_TYPE,
            buildErrorJson("Observable type, name or id args are missing!"));
        return true;
      }

      if (CallbacksManager.deleteCallback(type.c_str(), name.c_str(),
                                          id.toInt())) {
        CallbacksManager.saveCallbacksToSettings();
        server.send(200);
      } else {
        server.send(500, JSON_CONTENT_TYPE,
                    buildErrorJson("Failed to delete callback. Check logs for "
                                   "additional information."));
      }
      return true;
    }

    return false;
  };
};

#endif