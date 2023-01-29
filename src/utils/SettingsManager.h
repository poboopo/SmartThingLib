#include "DictionaryDeclarations.h"

#define SSID_SETTING "wssid"
#define PASSWORD_SETTING "wpass"

#define SETTINGS_MANAGER_TAG "settings_manager"

class SettingsManager {
    private:
        Dictionary * _settings;
        String loadFromEeprom();
    public:
        SettingsManager();
        ~SettingsManager();
        
        void loadSettings();
        void removeSetting(String name);
        void saveSettings();
        void clear();

        String getSetting(String name);
        String getJson();
        void mergeSettings(Dictionary dict);
};