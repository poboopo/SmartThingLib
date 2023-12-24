#ifndef HANDLER_UTILS_H
#define HANDLER_UTILS_H

#include <WebServer.h>

String buildErrorJson(String error) { return "{\"error\": \"" + error + "\"}"; }

#endif