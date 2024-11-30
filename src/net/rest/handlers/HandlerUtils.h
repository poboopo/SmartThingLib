#ifndef HANDLER_UTILS_H
#define HANDLER_UTILS_H

#include <Arduino.h>

#define ERROR_BODY_MISSING "{\"error\": \"Body is missing\"}"

String buildErrorJson(String error) { return "{\"error\": \"" + error + "\"}"; }

#endif