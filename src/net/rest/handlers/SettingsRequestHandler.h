#ifndef SETTINGS_RQ_H
#define SETTINGS_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/handlers/RequestHandler.h"

#define SETTINGS_RQ_PATH "/settings"
static const char * _SETTINGS_RQ_TAG = "settings_handler";

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
      String response = SettingsRepository.exportSettings();
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    if (request->method() == HTTP_POST) {
      if (_body.isEmpty()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, ERROR_BODY_MISSING);
      }
      st_log_debug(_SETTINGS_RQ_TAG, "Trying to import settings: %s", _body.c_str());
      if (SettingsRepository.importSettings(_body)) {
        st_log_debug(_SETTINGS_RQ_TAG, "Successfully imported settings!");
        st_log_warning(_SETTINGS_RQ_TAG, "Restarting in 5 sec!");
        delay(5000);
        ESP.restart();
        return request->beginResponse(200); // not reacheable
      } else {
        return request->beginResponse(500, CONTENT_TYPE_JSON, buildErrorJson("Settings import failed! Check logs for details"));
      }
    }
    return nullptr;
  }
};

#endif