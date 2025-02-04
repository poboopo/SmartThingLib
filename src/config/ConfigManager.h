#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "Features.h"

#if ENABLE_CONFIG

#include <Arduino.h>
#include <list>
#include <functional>
#include <ArduinoJson.h>

#define LOGGER_ADDRESS_CONFIG "laddr"
#define GATEWAY_CONFIG "gtw"
#define MAX_CONFIG_ENTRY_NAME_LENGTH 10

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

class ConfigManagerClass {
  public:
    ConfigManagerClass();
    ~ConfigManagerClass();

    /*
      Add config entry
      @param name unique system name
      @returns true if added
    */
    bool add(const char* name);
    /*
      Get config value
      @param name system name
      @returns config value
    */
    const char * get(const char * name);
    int getInt(const char * name, int defaultValue = 0);
    bool getBool(const char * name, bool defaultValue = false);
    /*
      Set config value
      @param name system name
      @param value new config value
      @returns true if value updated
    */
    bool set(const char * name, const char * value);
    /*
      Add config update handler
      @param handler lambda with handler logic
    */
    void onConfigUpdate(ConfigUpdatedHook handler);

    void loadConfigValues();
    bool setConfig(JsonDocument conf);
    bool dropConfig();
    String getConfigJson();
  private:
    std::list<ConfigEntry*> _config;
    ConfigUpdatedHook _configUpdatedHook = [](){};
  
    bool saveConfig();
    bool setConfigValueWithoutSave(const char * name, const char * value);
    void callConfigUpdateHook();
    std::list<ConfigEntry*>::iterator findConfigEntry(const char * name);
};

extern ConfigManagerClass ConfigManager;

#endif
#endif