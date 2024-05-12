#ifndef SETTINGS_RQ_H
#define SETTINGS_RQ_H

#include <WebServer.h>

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"

#define SETTINGS_RQ_PATH "/settings"
#define SETTINGS_RQ_TAG "settings_handler"

class SettingsRequestHandler : public RequestHandler {
 public:
  SettingsRequestHandler(){};
  bool canHandle(HTTPMethod method, String uri) {
    return uri.equals(SETTINGS_RQ_PATH) &&
           (method == HTTP_GET || method == HTTP_POST || method == HTTP_OPTIONS);
  };

  bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
    String body = server.arg("plain");
    LOGGER.logRequest(SETTINGS_RQ_TAG, http_method_str(requestMethod),
                      requestUri.c_str(), body.c_str());
    server.sendHeader("Access-Control-Allow-Origin", "*");

    if (requestMethod == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(200);
      return true;
    }
    if (requestMethod == HTTP_GET) {
      DynamicJsonDocument settings = STSettings.exportSettings();
      String response;
      serializeJson(settings, response);
      server.send(200, CONTENT_TYPE_JSON, response);
      return true;
    }
    if (requestMethod == HTTP_POST) {
      if (body.isEmpty()) {
        server.send(400, CONTENT_TYPE_JSON, ERROR_BODY_MISSING);
        return true;
      }
      DynamicJsonDocument doc(JSON_SETTINGS_DOC_SIZE);
      deserializeJson(doc, body);
      LOGGER.info(SETTINGS_RQ_TAG, "Trying to import settings: %s", body.c_str());
      if (STSettings.importSettings(doc)) {
        LOGGER.info(SETTINGS_RQ_TAG, "Successfully imported settings!");
        server.send(200);
        LOGGER.warning(SETTINGS_RQ_TAG, "Restarting in 5 sec!");
        delay(5000);
        ESP.restart();
      } else {
        LOGGER.error(SETTINGS_RQ_TAG, "Settings import failed");
        server.send(500, CONTENT_TYPE_JSON, buildErrorJson("Settings import failed! Check logs for details"));
      }
      return true;
    }

    return false;
  };
};

#endif