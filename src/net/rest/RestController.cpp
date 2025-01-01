#include "net/rest/RestController.h"

#include "Features.h"
#include "logs/BetterLogger.h"
#include "settings/SettingsRepository.h"
#include "sensors/SensorsManager.h"
#include "net/rest/handlers/ActionRequestHandler.h"
#include "net/rest/handlers/HooksRequestHandler.h"
#include "net/rest/handlers/ConfigRequestHandler.h"
#include "net/rest/handlers/InfoRequestHandler.h"
#include "net/rest/handlers/WiFiRequestHandler.h"
#include "net/rest/handlers/SettingsRequestHandler.h"
#include "net/rest/handlers/DangerRequestHandler.h"
#include "net/rest/handlers/SensorsRequestHandler.h"
#include "net/rest/handlers/AssetsRequestHandler.h"

const char * const _WEB_SERVER_TAG = "web_server";

String resetReasonAsString() {
  #ifdef ARDUINO_ARCH_ESP8266
  return ESP.getResetReason();
  #endif
  #ifdef ARDUINO_ARCH_ESP32
  switch (esp_reset_reason()) {
    case ESP_RST_POWERON:
      return "ESP_RST_POWERON";
    case ESP_RST_EXT:
      return "ESP_RST_EXT";
    case ESP_RST_SW:
      return "ESP_RST_SW";
    case ESP_RST_PANIC:
      return "ESP_RST_PANIC";
    case ESP_RST_INT_WDT:
      return "ESP_RST_INT_WDT";
    case ESP_RST_TASK_WDT:
      return "ESP_RST_TASK_WDT";
    case ESP_RST_WDT:
      return "ESP_RST_WDT";
    case ESP_RST_DEEPSLEEP:
      return "ESP_RST_DEEPSLEEP";
    case ESP_RST_BROWNOUT:
      return "ESP_RST_BROWNOUT";
    case ESP_RST_SDIO:
      return "ESP_RST_SDIO";
    default:
      return "ESP_RST_UNKNOWN";
  }
  #endif
}

RestControllerClass RestController;

RestControllerClass::RestControllerClass(): _server(AsyncWebServer(80)) {};
RestControllerClass::~RestControllerClass(){};

void RestControllerClass::begin() {
  if (_setupFinished) {
    end();
  }

  setupHandler();
  _server.begin();
  _setupFinished = true;
  st_log_info(_WEB_SERVER_TAG, "Web service started");
}

void RestControllerClass::end() {
  _setupFinished = false;
  _server.end();
  st_log_info(_WEB_SERVER_TAG, "Web service stopped");
}

void RestControllerClass::setupHandler() {
  _server.addHandler(new AssetsRequestHandler());
  _server.addHandler(new WiFiRequesthandler());
  _server.addHandler(new InfoRequestHandler());
  _server.addHandler(new SettingsRequestHandler());
  _server.addHandler(new DangerRequestHandler());

  #if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS
  _server.addHandler(new SensorsRequestHandler());
  #endif
  #if ENABLE_ACTIONS
  _server.addHandler(new ActionRequestHandler());
  #endif
  #if ENABLE_HOOKS
  _server.addHandler(new HooksRequestHandler());
  #endif
  #if ENABLE_CONFIG
    _server.addHandler(new ConfigRequestHandler());
  #endif

  _server.on("/health", HTTP_GET, [this](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "I am alive!!! :)");
  });

  _server.on("/features", HTTP_GET, [this](AsyncWebServerRequest * request) {
    JsonDocument doc;
    doc["web"] = ENABLE_WEB_PAGE == 1;
    doc["actions"] = ENABLE_ACTIONS == 1;
    doc["actionsScheduler"] = ENABLE_ACTIONS_SCHEDULER == 1;
    doc["sensors"] = ENABLE_NUMBER_SENSORS == 1 || ENABLE_TEXT_SENSORS == 1;
    doc["hooks"] = ENABLE_HOOKS == 1;
    doc["config"] = ENABLE_CONFIG == 1; 
    doc["logger"] = ENABLE_LOGGER == 1;
    
    String response;
    serializeJson(doc, response);
    AsyncWebServerResponse * resp = request->beginResponse(200, CONTENT_TYPE_JSON, response);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    request->send(resp);
  });

  _server.on("/metrics", HTTP_GET, [this](AsyncWebServerRequest * request) {
    JsonDocument doc;
    doc["uptime"] = millis();

    JsonObject obj = doc["heap"].to<JsonObject>();
    obj["free"] = ESP.getFreeHeap();
    doc["resetReason"] = resetReasonAsString();

    #ifdef ARDUINO_ARCH_ESP32
      obj["size"] = ESP.getHeapSize();
      obj["minFree"] = ESP.getMinFreeHeap();
      obj["maxAlloc"] = ESP.getMaxAllocHeap();
    #endif

    #if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS || ENABLE_HOOKS
      JsonObject counts = doc["counts"].to<JsonObject>();
      #if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS
        counts["sensors"] = SensorsManager.getSensorsCount();
      #endif
      #if ENABLE_HOOKS
        counts["hooks"] = HooksManager.getTotalHooksCount();
      #endif
    #endif

    String response;
    serializeJson(doc, response);
    AsyncWebServerResponse * resp = request->beginResponse(200, CONTENT_TYPE_JSON, response);
    resp->addHeader("Access-Control-Allow-Origin", "*");
    request->send(resp);
  });

  _server.onNotFound(
      [&](AsyncWebServerRequest * request) { request->send(404, "text/plain", "Page not found"); });
}