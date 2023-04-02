#include <ArduinoJson.h>
#include "net/BetterLogger.h"

#ifndef SettingsManager_H
#define SettingsManager_H

#define SSID_SETTING "ssid"
#define PASSWORD_SETTING "password"
#define AUTOMODE_SETTING "automode"

#define CLOSE_SETTING "light_close"
#define OPEN_SETTING "light_open"
#define BRIGHT_SETTING "light_bright"
#define DELAY_SETTING "delay"
#define ACCURACY_SETTING "accuracy"

#define GROUP_WIFI "wifi"
#define GROUP_CONFIG "config"
#define GROUP_STATE "state"

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