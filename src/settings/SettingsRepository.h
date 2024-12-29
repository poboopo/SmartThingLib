#ifndef SettingsRepository_H
#define SettingsRepository_H

#include <ArduinoJson.h>
#include <functional>

#include "logs/BetterLogger.h"
#include "utils/List.h"

#define LOGGER_ADDRESS_CONFIG "laddr"
#define GATEWAY_CONFIG "gtw"
#define MAX_CONFIG_ENTRY_NAME_LENGTH 10

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

class ConfigEntry {
  public:
    ConfigEntry(const char* name)
        : _value(nullptr) {
      _name = (char*) malloc(strlen(name) + 1);
      strcpy(_name, name);
    };
    ~ConfigEntry() {
      free(_name);
      if (_value != nullptr) {
        free(_value);
      }
    }

    const char * name() const {
      return _name;
    }
    const char * value() const {
      return _value == nullptr ? "" : _value;
    }

    void setValue(const char * value) {
      if (_value != nullptr) {
        free(_value);
        _value = nullptr;
      }

      if (value != nullptr && strlen(value) > 0) {
        _value = (char*) malloc(strlen(value) + 1);
        strcpy(_value, value);
      }
    }

  private:
    char* _name;
    char* _value;
};

typedef std::function<void(void)> ConfigUpdatedHook;

class SettingsRepositoryClass {
 public:
  SettingsRepositoryClass();
  ~SettingsRepositoryClass();

  String getName();
  bool setName(String name);

  WiFiConfig getWiFi();
  bool setWiFi(WiFiConfig settings);
  bool dropWiFi();

  void loadConfigValues();
  bool addConfigEntry(const char* name);
  const char * getConfigValue(const char * name) const;
  bool setConfigValue(const char * name, const char * value);
  bool setConfig(JsonDocument conf);
  bool dropConfig();
  String getConfigJson();
  void onConfigUpdate(ConfigUpdatedHook hook);

  #if ENABLE_HOOKS
  String getHooks();
  bool setHooks(String &data);
  bool dropHooks();
  #endif

  #if ENABLE_ACTIONS_SCHEDULER
  bool setActions(JsonDocument conf);
  JsonDocument getActions();
  #endif

  String exportSettings();
  bool importSettings(String &dump);
  
  void clear();
 private:
  List<ConfigEntry> _config;
  ConfigUpdatedHook _configUpdatedHook = [](){};
  
  bool saveConfig();
  bool setConfigValueWithoutSave(const char * name, const char * value);
  void callConfigUpdateHook();

  void read(uint16_t address, char * buff, uint16_t length);
  void write(uint16_t address, const char * buff, uint16_t length);

  int getLength(uint8_t index);
  int writeLength(uint8_t index, int length);

  String readData(uint8_t index, const char * defaultValue = "");
  int writeData(uint8_t index, const char * data);

  JsonDocument stringToObject(String& data);
  String objectToString(JsonDocument doc);
};

extern SettingsRepositoryClass SettingsRepository;

#endif