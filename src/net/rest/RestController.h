#ifndef REST_CONTROLLER_H
#define REST_CONTROLLER_H

#include <ESPAsyncWebServer.h>

#define CONTENT_TYPE_JSON "application/json"
#define CONTENT_TYPE_JS "text/javascript"

class RestControllerClass {
 public:
  RestControllerClass();
  ~RestControllerClass();
  void begin();
  void reload();

  AsyncWebServer* getWebServer() { return &_server; };
 private:
  bool _setupFinished = false;
  AsyncWebServer _server;

  void setupHandler();
};

extern RestControllerClass RestController;

#endif