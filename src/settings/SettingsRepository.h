#ifndef SettingsRepository_H
#define SettingsRepository_H

#include <ArduinoJson.h>
// #include <functional>

#include "logs/BetterLogger.h"

#define SSID_SETTING "ss"
#define PASSWORD_SETTING "ps"
#define WIFI_MODE_SETTING "md"
#define LOGGER_ADDRESS_CONFIG "laddr"
#define GATEWAY_CONFIG "gtw"

#define EEPROM_LOAD_SIZE 1024

struct WiFiConfig {
  String ssid;
  String password;
  uint8_t mode;
};

// typedef std::function<bool(JsonPair pair)> FilterFunction;

class SettingsRepositoryClass {
 private:
  void read(uint16_t address, char * buff, uint16_t length);
  void write(uint16_t address, const char * buff, uint16_t length);

  int getLength(uint8_t index);
  int writeLength(uint8_t index, int length);

  String readData(uint8_t index, const char * defaultValue = "");
  int writeData(uint8_t index, const char * data);

  JsonDocument stringToObject(String& data);
  String objectToString(JsonDocument doc);
 public:
  SettingsRepositoryClass();
  ~SettingsRepositoryClass();

  String getName();
  bool setName(String name);

  WiFiConfig getWiFi();
  bool setWiFi(WiFiConfig settings);
  bool dropWiFi();

  JsonDocument getConfig();
  bool setConfig(JsonDocument conf);
  bool dropConfig();

  #if ENABLE_HOOKS
  JsonDocument getHooks();
  bool setHooks(JsonDocument doc);
  bool dropHooks();
  #endif

  #if ENABLE_ACTIONS_SCHEDULER
  bool setActions(JsonDocument conf);
  JsonDocument getActions();
  #endif

  String exportSettings();
  bool importSettings(String dump);
  
  void clear();
};

extern SettingsRepositoryClass SettingsRepository;

#endif