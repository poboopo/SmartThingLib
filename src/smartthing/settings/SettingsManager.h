#ifndef SettingsManager_H
#define SettingsManager_H

#include <ArduinoJson.h>
#include "smartthing/logs/BetterLogger.h"

#define SSID_SETTING "ss"
#define PASSWORD_SETTING "ps"
#define WIFI_MODE_SETTING "md"

#define SETTINGS_MANAGER_TAG "settings_manager"
#define JSON_DOC_SIZE 2048
#define EEPROM_LOAD_SIZE 2048

class SettingsManager {
    private:
        DynamicJsonDocument _settings = DynamicJsonDocument(JSON_DOC_SIZE);
        const char * loadFromEeprom();
        bool _loaded = false;

        JsonObject getOrCreateObject(const char * name);
        void removeIfEmpty(const char * group);
        void addDefaultSettings();
    public:
        SettingsManager();
        ~SettingsManager();
        
        void loadSettings();
        void removeSetting(const char * name);
        void dropWifiCredits();
        void dropAll();
        void saveSettings();
        void clear();

        JsonObject getConfig();
        JsonObject getState();
        JsonObject getWiFi();
        const char * getDeviceName();
        void setDeviceName(const char * name);

        const JsonObject getAllSettings();
};

extern SettingsManager STSettings;

#endif