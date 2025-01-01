#ifndef ASSETS_RQ_H
#define ASSETS_RQ_H

#include <ESPAsyncWebServer.h>

#include "Features.h"
#include "net/rest/WebPageAssets.h"
#include "logs/BetterLogger.h"

const char * const _assetsUrl = "/assets";
const char * const _jsMediaType = "text/javascript";
const char * const _cssMediaType = "text/css";

class AssetsRequestHandler: public AsyncWebHandler {
  public:
    AssetsRequestHandler(){};
    virtual ~AssetsRequestHandler(){};

    bool canHandle(AsyncWebServerRequest *request) {
      return request->method() == HTTP_GET && 
        (request->url().equals("/") || request->url().startsWith(_assetsUrl));
    }

    void handleRequest(AsyncWebServerRequest *request) {      
      AsyncWebServerResponse * asyncResponse = processRequest(request);
      if (asyncResponse == nullptr) {
        asyncResponse = request->beginResponse(404);
      }

      asyncResponse->addHeader("Access-Control-Allow-Origin", "*");
      request->send(asyncResponse);
    }
  
  private:
    AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
      if (request->url().equals("/")) {
        return request->beginResponse_P(200, "text/html", WEB_PAGE_MAIN);
      }

      String resource = request->url().substring(strlen(_assetsUrl) + 1);
      if (resource.isEmpty()) {
        return nullptr;
      }

      st_log_debug("assets-handler", "Fetching resouce %s", resource.c_str());
      
      #if ENABLE_WEB_PAGE
        if (resource.equals("script.js")) {
          return request->beginResponse_P(200, _jsMediaType, SCRIPT_PAGE_MAIN);
        }
        if (resource.equals("styles.css")) {
          return request->beginResponse_P(200, _cssMediaType, STYLE_PAGE_MAIN);
        }
        if (resource.equals("sensors.js")) {
          return request->beginResponse_P(200, _jsMediaType, SCRIPT_SENSORS_TAB);
        }
        if (resource.equals("actions.js")) {
          return request->beginResponse_P(200, _jsMediaType, SCRIPT_ACTIONS_TAB);
        }
        if (resource.equals("config.js")) {
          return request->beginResponse_P(200, _jsMediaType, SCRIPT_CONFIG_TAB);
        }
        if (resource.equals("hooks.js")) {
          return request->beginResponse_P(200, _jsMediaType, SCRIPT_HOOKS_TAB);
        }
      #else
        if (resource.equals("minimal-script.js")) {
          return request->beginResponse_P(200, _jsMediaType, SCRIPT_PAGE_MAIN);
        }
      #endif

      st_log_error("assets-handler", "Unkown resource: %s", resource.c_str());
      return nullptr;
    }
};

#endif