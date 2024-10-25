#ifndef CONFIG_RH_H
#define CONFIG_RH_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsManager.h"
#include "settings/ConfigEntriesList.h"
#include "net/rest/handlers/RequestHandler.h"

#define CONFIG_PATH "/config"
static const char * CONFIG_LOG_TAG = "config_handler";

class ConfigRequestHandler : public RequestHandler {
 public:
  ConfigRequestHandler(RestHandlerFunction* configUpdatedHandler)
      : _configUpdatedHandler(configUpdatedHandler){};
  virtual ~ConfigRequestHandler() {};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(CONFIG_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_OPTIONS ||
            request->method() == HTTP_POST || request->method() == HTTP_DELETE);
  }

  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    String url = request->url();
    if (url.equals("/config/values")) {
      if (request->method() == HTTP_GET) {
        JsonObject config = STSettings.getConfig();
        String response;
        serializeJson(config, response);
        return request->beginResponse(200, CONTENT_TYPE_JSON, response);
      }
      if (request->method() == HTTP_POST) {
        Config::ConfigEntriesList * entriesList = SmartThing.getConfigInfo();
        if (entriesList->size() != 0) {
          JsonDocument jsonDoc;
          deserializeJson(jsonDoc, _body);
          JsonObject root = jsonDoc.as<JsonObject>();
          JsonObject config = STSettings.getConfig();

          for (JsonPair pair : root) {
            if (entriesList->haveConfigEntry(pair.key().c_str()) && !pair.value().isNull()) {
              config[pair.key()] = pair.value();
            } else {
              config.remove(pair.key());
            }
          }
          STSettings.save();
        }
        callHooks();
        return request->beginResponse(200);
      }
      if (request->method() == HTTP_DELETE) {
        if (!request->hasArg("name")) {
          return request->beginResponse(400, "content/json",
                      buildErrorJson("Setting name is missing"));
        }
        String name = request->arg("name");

        JsonObject config = STSettings.getConfig();
        if (config.containsKey(name)) {
          ST_LOG_WARNING(CONFIG_LOG_TAG, "Removing config value %s", name);
          config.remove(name);
          STSettings.save();
          callHooks();
          return request->beginResponse(200);
        } else {
          ST_LOG_ERROR(CONFIG_LOG_TAG, "Failed to remove config %s - no such key",
                      name);
          return request->beginResponse(404, "content/json", buildErrorJson("No such key"));
        }
      }
    }
    if (request->method() == HTTP_GET && url.equals("/config/info")) {
        JsonDocument doc = SmartThing.getConfigInfoJson();
        String response;
        serializeJson(doc, response);
        return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    
    if (request->method() == HTTP_DELETE) {
      if (request->url().equals("/config/delete/all")) {
        STSettings.dropConfig();
        STSettings.save();
        return request->beginResponse(200);
      }
      
    }
    return nullptr;
  }

 private:
  RestHandlerFunction* _configUpdatedHandler;
  
  void callHooks() {
    #if ENABLE_LOGGER
    LOGGER.updateAddress(STSettings.getConfig()[LOGGER_ADDRESS_CONFIG]);
    #endif
    if (_configUpdatedHandler != nullptr) {
      (*_configUpdatedHandler)();
    }
  }
};

#endif