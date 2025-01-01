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

const char * const _hooksSensorNameArg = "sensor";
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
        if (request->url().equals("/hooks/templates")) {
          String sensor = request->arg(_hooksSensorNameArg);
          if (sensor.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON,
                        buildErrorJson("Sensor sensor parameter are missing!"));
          }

          String response = HooksBuilder::getTemplates(sensor.c_str());
          return request->beginResponse(200, CONTENT_TYPE_JSON, response);
        }

        if (request->url().equals("/hooks/test")) {
          String sensor = request->arg(_hooksSensorNameArg);
          String id = request->arg(_hookIdArg);
          String value = request->arg("value");

          if (sensor.isEmpty() || id.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON,
                        buildErrorJson("Type, sensor or id args are missing!"));
          }
          st_log_info(_HOOKS_RQ_TAG, "Making test hook call for sensor %s, id = %s (value = %s)", sensor.c_str(), id.c_str(), value.isEmpty() ? "none" : value.c_str());
          if (HooksManager.callHook(sensor.c_str(), id.toInt(), value)) {
            return request->beginResponse(200);
          } else {
            return request->beginResponse(500);
          }
        }
      }

      if (request->url().equals(_hooksPath)) {
        if (request->method() == HTTP_GET) {
          String sensor = request->arg(_hooksSensorNameArg);

          if (sensor.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Sensor arg are missing!"));
          }

          st_log_debug(_HOOKS_RQ_TAG, "Searching hooks for sensor %s", sensor.c_str());
          JsonDocument doc = HooksManager.getSensorHooksJson(sensor.c_str());
          String response;
          serializeJson(doc, response);
          return request->beginResponse(200, CONTENT_TYPE_JSON, response);
        }

        // todo switch
        if (request->method() == HTTP_POST) {
          if (_body.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Body is missing!"));
          }

          JsonDocument doc;
          deserializeJson(doc, _body);

          if (!doc["sensor"].is<JsonVariant>() || !doc["hook"].is<JsonVariant>()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Sensor or hook object are missing"));
          }

          const char *sensor = doc["sensor"];
          if (sensor == nullptr) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Parameters sensor type or sensor are missing!"));
          }

          int id = HooksManager.addHook(sensor, doc["hook"].as<String>().c_str());

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
          String sensor = request->arg(_hooksSensorNameArg);
          String id = request->arg(_hookIdArg);

          if (sensor.isEmpty() || id.isEmpty()) {
            return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Sensor sensor or id args are missing!"));
          }

          if (HooksManager.deleteHook(sensor.c_str(), id.toInt())) {
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