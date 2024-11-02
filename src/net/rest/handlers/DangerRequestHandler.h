#ifndef DANGER_RQ_H
#define DANGER_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"

#define DANGER_RQ_PATH "/danger"
static const char * DANGER_RQ_TAG = "danger_handler";

class DangerRequestHandler : public AsyncWebHandler {
 public:
  DangerRequestHandler(){};
  virtual ~DangerRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(DANGER_RQ_PATH) &&
           (request->method() == HTTP_POST || request->method() == HTTP_OPTIONS);
  };

  void handleRequest(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type");
      request->send(response);
      return;
    }

    String url = request->url();
    st_log_request(DANGER_RQ_TAG, request->methodToString(), url.c_str(), "");

    if (url.equals("/danger/restart")) {
      st_log_request(DANGER_RQ_PATH, request->methodToString(), request->url().c_str(), "");

      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(200, "text/plain", "Restart in 2000ms");
      restart();
    }

    if (url.equals("/danger/wipe")) {
      st_log_request(DANGER_RQ_PATH, request->methodToString(), request->url().c_str(), "");

      ST_LOG_WARNING(DANGER_RQ_TAG, "Wiping all settings!");
      SettingsRepository.clear();

      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(200, "text/plain", "Settings wiped! Restart in 2000ms");
      restart();
    }
  };
 private:
  void restart() {
    delay(1500);
    ST_LOG_INFO(DANGER_RQ_TAG, "---------RESTART---------");
    delay(500);
    ESP.restart();
  }
};

#endif