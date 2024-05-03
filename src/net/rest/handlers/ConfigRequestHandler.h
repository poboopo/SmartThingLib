#ifndef CONFIG_RH_H
#define CONFIG_RH_H

#include <WebServer.h>

#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsManager.h"

#define CONFIG_PATH "/config"
#define CONFIG_LOG_TAG "config_handler"

class ConfigRequestHandler : public RequestHandler {
 public:
  ConfigRequestHandler(RestHandlerFunction* configUpdatedHandler)
      : _configUpdatedHandler(configUpdatedHandler){};

  bool canHandle(HTTPMethod method, String uri) {
    return uri.startsWith(CONFIG_PATH) &&
           (method == HTTP_GET || method == HTTP_OPTIONS ||
            method == HTTP_POST || method == HTTP_DELETE);
  }

  bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
    String body = server.arg("plain");
    LOGGER.logRequest(CONFIG_LOG_TAG, http_method_str(requestMethod),
                      requestUri.c_str(), body.c_str());

    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (requestMethod == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Methods",
                        "GET, POST, DELETE, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(200);
      return true;
    }
    if (requestMethod == HTTP_GET) {
      JsonObject config = STSettings.getConfig();
      String response;
      serializeJson(config, response);
      server.send(200, CONTENT_TYPE_JSON, response);
      return true;
    }
    if (requestMethod == HTTP_POST) {
      if (body.length() == 0) {
        server.send(400, CONTENT_TYPE_JSON, ERROR_BODY_MISSING);
        return true;
      }

      DynamicJsonDocument jsonDoc(1024);
      deserializeJson(jsonDoc, body);
      JsonObject root = jsonDoc.as<JsonObject>();
      JsonObject config = STSettings.getConfig();

      for (JsonPair pair : root) {
        config[pair.key()] = pair.value();
      }

      STSettings.save();
      server.send(200);
      callHooks();
      return true;
    }
    if (requestMethod == HTTP_DELETE) {
      if (requestUri.equals("/config/delete/all")) {
        STSettings.dropConfig();
        STSettings.save();
        server.send(200);
        return true;
      }
      if (!server.hasArg("name")) {
        server.send(400, "content/json",
                    buildErrorJson("Setting name is missing"));
      }
      String name = server.arg("name");

      JsonObject config = STSettings.getConfig();
      if (config.containsKey(name)) {
        LOGGER.warning(CONFIG_LOG_TAG, "Removing config value %s", name);
        config.remove(name);
        STSettings.save();
        server.send(200);
        callHooks();
      } else {
        LOGGER.error(CONFIG_LOG_TAG, "Failed to remove config %s - no such key",
                     name);
        server.send(404, "content/json", buildErrorJson("No such key"));
      }
      return true;
    }
    return false;
  }

 private:
  RestHandlerFunction* _configUpdatedHandler;
  void callHooks() {
    #if ENABLE_LOGGER
    LOGGER.configUpdateHook(STSettings.getConfig()[LOGGER_ADDRESS_CONFIG]);
    if (_configUpdatedHandler != nullptr) {
      (*_configUpdatedHandler)();
    }
    #endif
  }
};

#endif