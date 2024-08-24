#ifndef INFO_RQ_H
#define INFO_RQ_H

#include "SmartThing.h"
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"
#include "settings/SettingsManager.h"

#define INFO_RQ_PATH "/info"
#define INFO_RQ_TAG "wifi_handler"
#define MAX_CONTENT_LENGTH 1024

class InfoRequestHandler : public AsyncWebHandler {
 public:
  InfoRequestHandler(){};
  virtual ~InfoRequestHandler(){};

  bool canHandle(AsyncWebServerRequest *request) {
    return request->url().startsWith(INFO_RQ_PATH) &&
           (request->method() == HTTP_GET || request->method() == HTTP_PUT ||
            request->method() == HTTP_OPTIONS);
  }
  void handleRequest(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      AsyncWebServerResponse * response = request->beginResponse(200);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Allow-Methods", "GET, PUT, OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "Content-Type");
      request->send(response);
      return;
    }

    AsyncWebServerResponse * asyncResponse = processRequest(request);
    if (asyncResponse == nullptr) {
      LOGGER.error(INFO_RQ_TAG, "Response = nullptr!");
      request->send(500, CONTENT_TYPE_JSON, buildErrorJson("Internal error - failed to process request"));
    }
    asyncResponse->addHeader("Access-Control-Allow-Origin", "*");
    request->send(asyncResponse);
  }

 private:
  AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) {
    String body = request->arg("plain");
    LOGGER.logRequest(INFO_RQ_TAG, request->methodToString(), request->url().c_str(), body.c_str());

    if (request->url().equals("/info/system")) {
      if (request->method() == HTTP_GET) {
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["version"] = SMART_THING_VERSION;
        jsonDoc["name"] = SmartThing.getName();
        jsonDoc["type"] = SmartThing.getType();
        #ifdef ARDUINO_ARCH_ESP32
        jsonDoc["chip_model"] = ESP.getChipModel();
        jsonDoc["chip_revision"] = ESP.getChipRevision();
        #endif
        #ifdef ARDUINO_ARCH_ESP8266
        jsonDoc["chip_model"] = "unknown";
        jsonDoc["chip_revision"] = "unknown";
        #endif

        String result;
        serializeJson(jsonDoc, result);
        return request->beginResponse(200, CONTENT_TYPE_JSON, result);
      }
      if (request->method() == HTTP_PUT) {
        if (body.isEmpty()) {
          return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Body is missing!"));
        }
        DynamicJsonDocument jsDoc(64);
        deserializeJson(jsDoc, body);
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
    if (request->url().equals("/info/actions") && request->method() == HTTP_GET) {
      #if ENABLE_ACTIONS 
      DynamicJsonDocument doc = SmartThing.getActionsInfo();
      String response;
      serializeJson(doc, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
      #else
      return request->beginResponse(400, CONTENT_TYPE_JSON, buildErrorJson("Actions feature disabled"));
      #endif
    }
    if (request->url().equals("/info/config") && request->method() == HTTP_GET) {
      DynamicJsonDocument doc = SmartThing.getConfigInfoJson();
      String response;
      serializeJson(doc, response);
      return request->beginResponse(200, CONTENT_TYPE_JSON, response);
    }
    return nullptr;
  }
};

#endif