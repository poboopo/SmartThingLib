#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <Arduino.h>
#include "settings/SettingsManager.h"
#include "logs/BetterLogger.h"

#ifdef ARDUINO_ARCH_ESP32
#define VALUE_DYNAMIC_PARAM "v"
#endif
#ifdef ARDUINO_ARCH_ESP8266
#define VALUE_DYNAMIC_PARAM "{v}"
#endif

// replaces keys in string with config values
inline String replaceValues(const char * input, String &value) {
  #ifdef ARDUINO_ARCH_ESP32
  JsonObject conf = STSettings.getConfig();

  String result = "";
  String key = "";
  bool opened = false;

  for (int i = 0; i < strlen(input); i++) {
    if (input[i] == '{') {
      if (opened) {
        result += "{" + key;
        key.clear();
      }
      opened = true;
    } else if (opened) {
      if (input[i] == '}') {
        if (key.equals(VALUE_DYNAMIC_PARAM)) {
          result += value;
        } else if (conf.containsKey(key)) {
          result += conf[key].as<String>();
        }
        opened = false;
        key.clear();
      } else {
        key += input[i];
      }
    } else {
      result += input[i];
    }
  }
  if (opened && key.length() > 0) {
    result += "{" + key;
  }
  return result;
  #endif
  #ifdef ARDUINO_ARCH_ESP8266
  String result = input;
  result.replace(VALUE_DYNAMIC_PARAM, value);
  return result;
  #endif
}

#endif