#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <Arduino.h>
#include "config/ConfigManager.h"
#include "logs/BetterLogger.h"

#ifdef ARDUINO_ARCH_ESP32
#define VALUE_DYNAMIC_PARAM "v"
#endif
#ifdef ARDUINO_ARCH_ESP8266
#define VALUE_DYNAMIC_PARAM "{v}"
#endif

inline bool trimUrl(String &url) {
  url.trim();
  if (url.startsWith("https://")) {
    url.remove(0, 8);
    return true;
  }
  if (url.startsWith("http://")) {
    url.remove(0, 7);
  }
  return false;
}
// replaces keys in string with config values
// not optimal impl, but it ok for now
inline String replaceValues(const char * input, String &value) {
  String result = "";
  String key = "";
  bool opened = false;

  for (int i = 0; i < (int) strlen(input); i++) {
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
        } 
        #if ENABLE_CONFIG
          else {
            result += ConfigManager.getConfigValue(key.c_str());
          }
        #endif
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
}

#endif