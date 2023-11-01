#ifndef SettingsManager_H
#define SettingsManager_H

#include <ArduinoJson.h>
#include "logs/BetterLogger.h"

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
        void clear(); //clears EEPROM
    public:
        SettingsManager();
        ~SettingsManager();
        
        void loadSettings();
        void removeSetting(const char * name);
        void dropAll();
        void save();

        JsonObject getConfig();
        void dropConfig();
        
        JsonObject getState();
        
        JsonObject getWiFi();
        void dropWifiCredits();
        
        String getDeviceName();
        void setDeviceName(const char * name);

        void setCallbacks(JsonArray doc);
        JsonArray getCallbacks();
        void dropAllCallbacks();

        const DynamicJsonDocument getAllSettings();
        int16_t usage() {
            return _settings.memoryUsage();
        }
};

extern SettingsManager STSettings;

#endif