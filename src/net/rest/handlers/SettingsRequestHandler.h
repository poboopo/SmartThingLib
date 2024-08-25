#ifndef SETTINGS_RQ_H
#define SETTINGS_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/handlers/RequestHandler.h"

#define SETTINGS_RQ_PATH "/settings"
#define SETTINGS_RQ_TAG "settings_handler"

class SettingsRequestHandler : public RequestHandler {
 public:
  SettingsRequestHandler(){};
  virtual ~SettingsRequestHandler(){};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().equals(SETTINGS_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_POST ||
           request->method() == HTTP_OPTIONS);
  };

  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    if (request->method() == HTTP_GET) {
      JsonDocument settings = STSettings.exportSettings();
      String response;
      serializeJson(settings, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    if (request->method() == HTTP_POST) {
      if (_body.isEmpty()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, ERROR_BODY_MISSING);
      }
      JsonDocument doc;
      deserializeJson(doc, _body);
      LOGGER.info(SETTINGS_RQ_TAG, "Trying to import settings: %s", _body.c_str());
      if (STSettings.importSettings(doc)) {
        LOGGER.info(SETTINGS_RQ_TAG, "Successfully imported settings!");
        return request->beginResponse(200);
        LOGGER.warning(SETTINGS_RQ_TAG, "Restarting in 5 sec!");
        delay(5000);
        ESP.restart();
      } else {
        LOGGER.error(SETTINGS_RQ_TAG, "Settings import failed");
        return request->beginResponse(500, CONTENT_TYPE_JSON, buildErrorJson("Settings import failed! Check logs for details"));
      }
    }
    return nullptr;
  }
};

#endif