#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <ESPAsyncWebServer.h>
#include "logs/BetterLogger.h"
#include "net/rest/RestController.h"
#include "net/rest/handlers/HandlerUtils.h"

static const char * REQUEST_HANDLER_TAG = "request";

class RequestHandler : public AsyncWebHandler {
  public:
    RequestHandler() {};
    virtual ~RequestHandler() {};

    void handleRequest(AsyncWebServerRequest *request) {
      if (request->method() == HTTP_OPTIONS) {
        AsyncWebServerResponse * response = request->beginResponse(200);
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");
        request->send(response);
        return;
      }

      LOGGER.logRequest(REQUEST_HANDLER_TAG, request->methodToString(), request->url().c_str(), _body.c_str());
      AsyncWebServerResponse * asyncResponse = processRequest(request);
      if (asyncResponse == nullptr) {
        SMT_LOG_ERROR(REQUEST_HANDLER_TAG, "Response = nullptr!");
        request->send(500, CONTENT_TYPE_JSON, buildErrorJson("Internal error - failed to process request"));
      }
      asyncResponse->addHeader("Access-Control-Allow-Origin", "*");
      request->send(asyncResponse);
      if (!_body.isEmpty()) {
        _body.clear();
      }
    }

    void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      for(size_t i=0; i<len; i++){
        _body += (char) data[i];
      }
    }

    virtual AsyncWebServerResponse * processRequest(AsyncWebServerRequest * request) { return nullptr; };
  protected:
    String _body;
};

#endif