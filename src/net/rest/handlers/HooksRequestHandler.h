#ifndef HOOKS_RQ_H
#define HOOKS_RQ_H

#include "Features.h"
#if ENABLE_HOOKS 

#include <ArduinoJson.h>

#include "SmartThing.h"
#include "hooks/builders/HooksBuilder.h"
#include "logs/BetterLogger.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "net/rest/handlers/RequestHandler.h"
#include "net/rest/WebPageAssets.h"

const char * const _hooksObsNameArg = "name";
const char * const _hooksObsTypeArg = "type";
const char * const _hookIdArg = "id";
const char * const _hooksPath = "/hooks";

const char * const _HOOKS_RQ_TAG = "hooks_handler";

// todo split to methods
class HooksRequestHandler : public RequestHandler {
  public:
    HooksRequestHandler(){};
    bool canHandle(AsyncWebServerRequest *request) {
      return request->url().startsWith(_hooksPath) &&
            (request->method() == HTTP_GET || request->method() == HTTP_PUT || request->method() == HTTP_POST ||
              request->method() == HTTP_DELETE || request->method() == HTTP_OPTIONS);
    };
    AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
      if (request->method() == HTTP_GET) {
        #if ENABLE_WEB_PAGE
        if (request->url().equals("/hooks/script.js")) {
          return request->beginResponse(200, CONTENT_TYPE_JS, SCRIPT_HOOKS_TAB);
        }
        #endif
        if (request->url().equals("/hooks/templates")) {
          String type = request->arg(_hooksObsTypeArg);
          if (type.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON,
                        buildErrorJson("Type parameter are missing!"));
          }
          String response = HooksBuilder::getTemplates(observableTypeFromStr(type.c_str()));;
          return request->beginResponse(200, CONTENT_TYPE_JSON, response);
        }
        if (request->url().equals("/hooks/by/observable")) {
          String type = request->arg(_hooksObsTypeArg);
          String name = request->arg(_hooksObsNameArg);

          if (type.isEmpty() || name.isEmpty()) {
            return request->beginResponse(
                400, CONTENT_TYPE_JSON,
                buildErrorJson("Observable type or name args are missing!"));
          }
          st_log_info(_HOOKS_RQ_TAG, "Searching hooks for [%s] %s", type.c_str(), name.c_str());
          JsonDocument doc = HooksManager.getObservableHooksJson(type.c_str(), name.c_str());
          String response;
          serializeJson(doc, response);
          return request->beginResponse(200, CONTENT_TYPE_JSON, response);
        }
        if (request->url().equals("/hooks/test")) {
          String type = request->arg(_hooksObsTypeArg);
          String name = request->arg(_hooksObsNameArg);
          String id = request->arg(_hookIdArg);
          String value = request->arg("value");

          if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON,
                        buildErrorJson("Type, name or id args are missing!"));
          }
          st_log_info(_HOOKS_RQ_TAG, "Making test hook call for [%s] %s, id = %s (value = %s)", type.c_str(), name.c_str(), id.c_str(), value.isEmpty() ? "none" : value.c_str());
          if (HooksManager.callHook(type.c_str(), name.c_str(), id.toInt(), value)) {
            return request->beginResponse(200);
          } else {
            return request->beginResponse(500);
          }
        }
      }
      if (request->url().equals(_hooksPath)) {
        // todo switch
        if (request->method() == HTTP_POST) {
          if (_body.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Body is missing!"));
          }

          JsonDocument doc;
          deserializeJson(doc, _body);

          if (!doc["observable"].is<JsonVariant>() || !doc["hook"].is<JsonVariant>()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Observable or hook object are missing"));
          }

          ObservableType type = observableTypeFromStr(doc["observable"][_hooksObsTypeArg]);
          const char *name = doc["observable"][_hooksObsNameArg];
          if (type == UNKNOWN_OBS_TYPE || name == nullptr) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Parameters observable type or name are missing!"));
          }

          int id = HooksManager.addHook(type, name, doc["hook"].as<String>().c_str());

          if (id >= 0) {
            HooksManager.saveInSettings();
            String response = "{\"id\":" + String(id) + "}";
            return request->beginResponse(201, CONTENT_TYPE_JSON, response);
          } else {
            return request->beginResponse(500, CONTENT_TYPE_JSON,
                        buildErrorJson("Failed to create hook. Check logs for "
                                      "additional information."));
          }
        }
        if (request->method() == HTTP_PUT) {
          if (_body.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Body is missing!"));
          }

          JsonDocument doc;
          deserializeJson(doc, _body);
          if (HooksManager.updateHook(doc)) {
            HooksManager.saveInSettings();
            return request->beginResponse(200);
          } else {
            return request->beginResponse(500, CONTENT_TYPE_JSON,
                        buildErrorJson("Failed to update hook. Check logs for "
                                      "additional information."));
          }
        }
        if (request->method() == HTTP_DELETE) {
          String type = request->arg(_hooksObsTypeArg);
          String name = request->arg(_hooksObsNameArg);
          String id = request->arg(_hookIdArg);

          if (type.isEmpty() || name.isEmpty() || id.isEmpty()) {
            return request->beginResponse(
                400, CONTENT_TYPE_JSON,
                buildErrorJson("Observable type, name or id args are missing!"));
          }

          if (HooksManager.deleteHook(type.c_str(), name.c_str(), id.toInt())) {
            HooksManager.saveInSettings();
            return request->beginResponse(200);
          } else {
            return request->beginResponse(500, CONTENT_TYPE_JSON,
                        buildErrorJson("Failed to delete hook. Check logs for "
                                      "additional information."));
          }
        }
      }
      
      return nullptr;
    }
};
#endif

#endif