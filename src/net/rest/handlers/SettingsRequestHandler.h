#ifndef SETTINGS_RQ_H
#define SETTINGS_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"

#define SETTINGS_RQ_PATH "/settings"
#define SETTINGS_RQ_TAG "settings_handler"

class SettingsRequestHandler : public AsyncWebHandler {
 public:
  SettingsRequestHandler(){};
  virtual ~SettingsRequestHandler(){};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().equals(SETTINGS_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_POST ||
           request->method() == HTTP_OPTIONS);
  };

  void handleRequest(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
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
    String body = request->arg("plain");
    LOGGER.logRequest(SETTINGS_RQ_TAG, request->methodToString(), request->url().c_str(), body.c_str());

    if (request->method() == HTTP_GET) {
      DynamicJsonDocument settings = STSettings.exportSettings();
      String response;
      serializeJson(settings, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    if (request->method() == HTTP_POST) {
      if (body.isEmpty()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, ERROR_BODY_MISSING);
      }
      DynamicJsonDocument doc(JSON_SETTINGS_DOC_SIZE);
      deserializeJson(doc, body);
      LOGGER.info(SETTINGS_RQ_TAG, "Trying to import settings: %s", body.c_str());
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