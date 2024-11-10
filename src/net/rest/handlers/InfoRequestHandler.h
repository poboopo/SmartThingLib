#ifndef INFO_RQ_H
#define INFO_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsRepository.h"
#include "net/rest/handlers/RequestHandler.h"

#define INFO_RQ_PATH "/info"

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
        #ifdef __VERSION
        jsonDoc["version"] = __VERSION;
        #endif
        jsonDoc["stVersion"] = SMART_THING_VERSION;
        jsonDoc["name"] = SmartThing.getName();
        jsonDoc["type"] = SmartThing.getType();
        jsonDoc["ip"] = SmartThing.getIp();
        #ifdef ARDUINO_ARCH_ESP32
        jsonDoc["board"] = "esp32";
        #endif
        #ifdef ARDUINO_ARCH_ESP8266
        jsonDoc["board"] = "esp8266";
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
    return nullptr;
  }
};

#endif