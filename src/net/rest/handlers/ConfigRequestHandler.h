#ifndef CONFIG_RH_H
#define CONFIG_RH_H

#include "Features.h"

#if ENABLE_CONFIG

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsRepository.h"
#include "net/rest/handlers/RequestHandler.h"

#define CONFIG_PATH "/config"
const char * const _CONFIG_LOG_TAG = "config_handler";

class ConfigRequestHandler : public RequestHandler {
 public:
  ConfigRequestHandler() {};
  virtual ~ConfigRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(CONFIG_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_OPTIONS ||
            request->method() == HTTP_POST || request->method() == HTTP_DELETE);
  }

  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    if (request->url().equals(CONFIG_PATH)) {
      if (request->method() == HTTP_GET) {
        String response = SettingsRepository.getConfigJson();
        return request->beginResponse(200, CONTENT_TYPE_JSON, response);
      }

      if (request->method() == HTTP_POST) {
        JsonDocument jsonDoc;
        deserializeJson(jsonDoc, _body);
        SettingsRepository.setConfig(jsonDoc);
        return request->beginResponse(200);
      }

      if (request->method() == HTTP_DELETE) {
        String name = request->arg("name");
        if (name.isEmpty()) {
          return request->beginResponse(400, "content/json", buildErrorJson("Config name is missing"));
        }

        if (SettingsRepository.setConfigValue(name.c_str(), nullptr)) {
          return request->beginResponse(200);
        } else {
          return request->beginResponse(404, "content/json", buildErrorJson("No such config entry"));
        }
      }
    }
    
    if (request->method() == HTTP_DELETE && request->url().equals("/config/delete/all")) {
      SettingsRepository.dropConfig();
      return request->beginResponse(200);
    }

    return nullptr;
  }
};

#endif
#endif