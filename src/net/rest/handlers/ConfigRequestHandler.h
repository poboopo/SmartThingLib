#ifndef CONFIG_RH_H
#define CONFIG_RH_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsRepository.h"
#include "settings/ConfigEntriesList.h"
#include "net/rest/handlers/RequestHandler.h"

#define CONFIG_PATH "/config"
static const char * _CONFIG_LOG_TAG = "config_handler";

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
        JsonDocument config = SettingsRepository.getConfig();
        ConfigEntriesList * list = SettingsRepository.getConfigInfo();
        JsonObject obj = config.as<JsonObject>();

        // jest'
        // hard typization required only over rest, that's why
        list->forEach([obj](ConfigEntry * entry) {
          if (entry->type != CONFIG_STRING && obj[entry->name].is<const char*>()) {
            if (entry->type == CONFIG_INTEGER) {
              obj[entry->name] = obj[entry->name].as<int>();
            } else if (entry->type == CONFIG_BOOLEAN) {
              obj[entry->name] = obj[entry->name].as<String>().equals("true");
            }
          }
        });

        String response;
        serializeJson(config, response);
        return request->beginResponse(200, CONTENT_TYPE_JSON, response);
      }
      if (request->method() == HTTP_POST) {
        JsonDocument jsonDoc;
        deserializeJson(jsonDoc, _body);
        SettingsRepository.setConfig(jsonDoc);
        callHooks();
        return request->beginResponse(200);
      }
      if (request->method() == HTTP_DELETE) {
        if (!request->hasArg("name")) {
          return request->beginResponse(400, "content/json",
                      buildErrorJson("Setting name is missing"));
        }
        String name = request->arg("name");

        JsonDocument config = SettingsRepository.getConfig();
        if (config[name].is<JsonVariant>()) {
          st_log_warning(_CONFIG_LOG_TAG, "Removing config value %s", name);
          config.remove(name);
          SettingsRepository.setConfig(config);
          callHooks();
          return request->beginResponse(200);
        } else {
          st_log_error(_CONFIG_LOG_TAG, "Failed to remove config %s - no such key", name);
          return request->beginResponse(404, "content/json", buildErrorJson("No such key"));
        }
      }
    }
    if (request->method() == HTTP_GET && url.equals("/config/info")) {
        JsonDocument doc = SettingsRepository.getConfigInfoJson();
        String response;
        serializeJson(doc, response);
        return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    
    if (request->method() == HTTP_DELETE) {
      if (request->url().equals("/config/delete/all")) {
        SettingsRepository.dropConfig();
        return request->beginResponse(200);
      }
    }
    return nullptr;
  }

 private:
  RestHandlerFunction* _configUpdatedHandler;
  
  void callHooks() {
    #if ENABLE_LOGGER
    LOGGER.updateAddress(SettingsRepository.getConfig()[LOGGER_ADDRESS_CONFIG]);
    #endif
    if (_configUpdatedHandler != nullptr) {
      (*_configUpdatedHandler)();
    }
  }
};

#endif