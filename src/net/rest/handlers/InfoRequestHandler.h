#ifndef INFO_RQ_H
#define INFO_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsManager.h"
#include "net/rest/handlers/RequestHandler.h"

#define INFO_RQ_PATH "/info"
#define INFO_RQ_TAG "info_handler"

class InfoRequestHandler : public RequestHandler {
 public:
  InfoRequestHandler(){};
  virtual ~InfoRequestHandler(){};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(INFO_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_PUT ||
            request->method() == HTTP_OPTIONS);
  }

  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    if (request->url().equals("/info/system")) {
      if (request->method() == HTTP_GET) {
        JsonDocument jsonDoc;
        jsonDoc["version"] = SMART_THING_VERSION;
        jsonDoc["name"] = SmartThing.getName();
        jsonDoc["type"] = SmartThing.getType();
        #ifdef ARDUINO_ARCH_ESP32
        jsonDoc["platform"] = "esp32";
        jsonDoc["chip_model"] = ESP.getChipModel();
        jsonDoc["chip_revision"] = ESP.getChipRevision();
        #endif
        #ifdef ARDUINO_ARCH_ESP8266
        jsonDoc["platform"] = "esp8266";
        jsonDoc["chip_model"] = "unknown";
        jsonDoc["chip_revision"] = "unknown";
        #endif

        String result;
        serializeJson(jsonDoc, result);
        return request->beginResponse(200, CONTENT_TYPE_JSON, result);
      }
      if (request->method() == HTTP_PUT) {
        if (_body.isEmpty()) {
          return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Body is missing!"));
        }
        JsonDocument jsDoc;
        deserializeJson(jsDoc, _body);
        const char* newName = jsDoc["name"];
        if (strlen(newName) == 0 || strlen(newName) > DEVICE_NAME_LENGTH_MAX) {
          return request->beginResponse(
              400, CONTENT_TYPE_JSON,
              buildErrorJson("Name is missing or too long (max 10 symbols)"));
        }

        SmartThing.updateDeviceName(newName);
        return request->beginResponse(200);
      }
    }
    // why it's here???
    // todo move to actions handler 
    if (request->url().equals("/info/actions") && request->method() == HTTP_GET) {
      #if ENABLE_ACTIONS 
      JsonDocument doc = SmartThing.getActionsInfo();
      String response;
      serializeJson(doc, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
      #else
      return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Actions feature disabled"));
      #endif
    }
    if (request->url().equals("/info/config") && request->method() == HTTP_GET) {
      JsonDocument doc = SmartThing.getConfigInfoJson();
      String response;
      serializeJson(doc, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    return nullptr;
  }
};

#endif