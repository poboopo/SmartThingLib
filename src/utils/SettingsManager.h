#ifndef SettingsManager_H
#define SettingsManager_H

#include <ArduinoJson.h>
#include "net/logs/BetterLogger.h"

#define DEVICE_NAME "dn"

#define SSID_SETTING "ss"
#define PASSWORD_SETTING "ps"
#define WIFI_MODE_SETTING "md"

// TODO move to main, not lib const
#define GROUP_WIFI "wf"
#define GROUP_CONFIG "cg"
#define GROUP_STATE "st"
#define AUTOMODE_SETTING "am"

#define SETTINGS_MANAGER_TAG "settings_manager"
#define JSON_DOC_SIZE 2048

class SettingsManager {
    private:
        static StaticJsonDocument<JSON_DOC_SIZE> _settings;
        static const char * loadFromEeprom();
        static bool _loaded;
    public:
        SettingsManager();
        ~SettingsManager();
        
        static void loadSettings();
        static void removeSetting(String name);
        static void dropWifiCredits();
        static void dropAll();
        static void saveSettings();
        static void clear();

        static void putSetting(String name, String value);
        static void putSetting(String groupName, String name, String value);
        static void putSetting(String name, int value);
        static void putSetting(String groupName, String name, int value);

        static void putSetting(String groupName, String name, JsonVariant var);
        static void putSetting(String groupName, JsonObject jsonObject);

        static const String getSettingString(String name);
        static const String getSettingString(String groupName, String name);

        static const int getSettingInteger(String name);
        static const int getSettingInteger(String groupName, String name);

        static const JsonObject getSettings();
        static const JsonObject getSettings(String groupName);

        static const String getJson();
        static const String getJson(String groupName);
};

#endif