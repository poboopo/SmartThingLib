#ifndef SettingsRepository_H
#define SettingsRepository_H

#include <ArduinoJson.h>
#include <functional>
#include <list>

#include "Features.h"
#include "logs/BetterLogger.h"

enum StWiFiMode {
  ST_WIFI_STA = 1,
  ST_WIFI_AP = 2,
  ST_WIFI_STA_TO_AP = 5
};

struct WiFiConfig {
  String ssid;
  String password;
  StWiFiMode mode;
};

class SettingsRepositoryClass {
 public:
  SettingsRepositoryClass();
  ~SettingsRepositoryClass();

  String getName();
  bool setName(String name);

  WiFiConfig getWiFi();
  bool setWiFi(WiFiConfig &settings);

  #if ENABLE_CONFIG
    String getConfig();
    bool setConfig(const String &config);
  #endif

  #if ENABLE_HOOKS
    String getHooks();
    bool setHooks(const String &data);
  #endif

  #if ENABLE_ACTIONS_SCHEDULER
    bool setActions(const JsonDocument &conf);
    JsonDocument getActions();
  #endif

  String exportSettings();
  bool importSettings(String &dump);
  
  void clear();
 private:
  void read(uint16_t address, char * buff, uint16_t length);
  void write(uint16_t address, const char * buff, uint16_t length);

  int getLength(uint8_t index);
  int writeLength(uint8_t index, int length);

  String readData(uint8_t index, const char * defaultValue = "");
  int writeData(uint8_t index, const char * data);
  /*
  Helper method for writeData
  Checks writes data length, prints messages
  */
  bool setData(uint8_t index, const char * data, const char * name, size_t expectedLength = 0);

  JsonDocument stringToObject(String& data);
  String objectToString(JsonDocument doc);
};

extern SettingsRepositoryClass SettingsRepository;

#endif