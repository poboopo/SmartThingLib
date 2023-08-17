#ifndef CALLBACKS_TEMPLATES_H
#define CALLBACKS_TEMPLATES_H

#include <Arduino.h>

const String CALLBACKS_TEMPLATES_JSON = R"=====(
{
  "http_callback": {
    "url": {
      "required": true,
    },
    "method": {
      "required": false,
      "values": [
        "GET",
        "POST",
        "DELETE",
        "PUT"
      ]
    },
    "payload": {
      "required": false
    }
  }
}
)=====";

#endif