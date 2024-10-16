#ifndef WIFI_RQ_H
#define WIFI_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsManager.h"
#include "net/rest/handlers/RequestHandler.h"

#define WIFI_RQ_PATH "/wifi"

class WiFiRequesthandler : public RequestHandler {
 public:
  WiFiRequesthandler() {};
  virtual ~WiFiRequesthandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(WIFI_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_POST ||
            request->method() == HTTP_OPTIONS);
  }

  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    if (request->method() == HTTP_GET) {
      JsonDocument jsonDoc;
      jsonDoc["settings"] = STSettings.getWiFi();
      JsonObject modes = jsonDoc["modes"].to<JsonObject>();
      modes[String(WIFI_MODE_STA)] = "STA";
      modes[String(WIFI_MODE_AP)] = "AP";

      String response;
      serializeJson(jsonDoc, response);

      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    } else if (request->method() == HTTP_POST) {
      if (_body.length() == 0) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, ERROR_BODY_MISSING);
      }

      JsonDocument jsonDoc;
      deserializeJson(jsonDoc, _body);

      String ssid = jsonDoc["ssid"].as<String>();
      if (ssid.isEmpty()) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Ssid is missing"));
      }
      if (ssid.length() > 32) {
        return request->beginResponse(400, CONTENT_TYPE_JSON,  buildErrorJson("Ssid is too long (32 symbols max)"));
      }
      String password = jsonDoc["password"].as<String>();
      if (password.length() > 0 && password.length() < 8) {
        return request->beginResponse(
            400, CONTENT_TYPE_JSON,
            buildErrorJson("Password is too short (8 symbols or blank)"));
      }
      int mode = jsonDoc["mode"].as<int>();
      if (mode < 0) {
        return request->beginResponse(400, CONTENT_TYPE_JSON,
                    buildErrorJson("WiFi mode can't be negative"));
      }
      if (mode != WIFI_MODE_AP && mode != WIFI_MODE_STA) {
        return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Unkown wifi mode"));
      }

      JsonObject wifiSettings = STSettings.getWiFi();
      wifiSettings[SSID_SETTING] = ssid;
      wifiSettings[PASSWORD_SETTING] = password;
      wifiSettings[WIFI_MODE_SETTING] = mode;
      STSettings.save();
      return request->beginResponse(200);
    }
    return nullptr;
  }
};

#endif