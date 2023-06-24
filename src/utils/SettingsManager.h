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

class SettingsManager {
    private:
        StaticJsonDocument<1024> _settings;
        String loadFromEeprom();
    public:
        SettingsManager();
        ~SettingsManager();
        
        void loadSettings();
        void removeSetting(String name);
        void dropWifiCredits();
        void dropAll();
        void saveSettings();
        void clear();

        void putSetting(String name, String value);
        void putSetting(String groupName, String name, String value);
        void putSetting(String name, int value);
        void putSetting(String groupName, String name, int value);

        void putSetting(String groupName, String name, JsonVariant var);
        void putSetting(String groupName, JsonObject jsonObject);

        String getSettingString(String name);
        String getSettingString(String groupName, String name);

        int getSettingInteger(String name);
        int getSettingInteger(String groupName, String name);

        JsonObject getSettings();
        JsonObject getSettings(String groupName);

        String getJson();
        String getJson(String groupName);
};

#endif