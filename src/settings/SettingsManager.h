#ifndef SettingsManager_H
#define SettingsManager_H

#include <ArduinoJson.h>

#include "logs/BetterLogger.h"

#define SSID_SETTING "ss"
#define PASSWORD_SETTING "ps"
#define WIFI_MODE_SETTING "md"
#define LOGGER_ADDRESS_CONFIG "laddr"
#define GATEWAY_CONFIG "gtw"

#define SETTINGS_MANAGER_TAG "settings_manager"

#define JSON_SETTINGS_DOC_SIZE 2048
#define EEPROM_LOAD_SIZE 1024

class SettingsManager {
 private:
  DynamicJsonDocument _settings = DynamicJsonDocument(JSON_SETTINGS_DOC_SIZE);
  String loadFromEeprom();
  bool _loaded = false;

  JsonObject getOrCreateObject(const char* name);
  void removeIfEmpty(const char* group);
  void addDefaultSettings();
  void clear();
 public:
  SettingsManager();
  ~SettingsManager();

  void loadSettings();
  void removeSetting(const char* name);
  void dropAll();
  bool save();

  JsonObject getConfig();
  void dropConfig();

  JsonObject getWiFi();
  void dropWifiCredits();

  const String getDeviceName();
  void setDeviceName(const char* name);

  void setHooks(JsonArray doc);
  JsonArray getHooks();
  void dropAllHooks();

  const DynamicJsonDocument exportSettings();
  bool importSettings(DynamicJsonDocument doc);

  const DynamicJsonDocument getAllSettings();
  int16_t usage() { return _settings.memoryUsage(); }
};

extern SettingsManager STSettings;

#endif