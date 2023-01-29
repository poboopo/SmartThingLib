#include <utils/SettingsManager.h>
#include <EEPROM.h>

#define EEPROM_LOAD_SIZE 2048
SettingsManager::SettingsManager() {
}

SettingsManager::~SettingsManager() {
    _settings->destroy();
}

void SettingsManager::loadSettings() {
    _settings = new Dictionary();
    ESP_LOGI(SETTINGS_MANAGER_TAG, "Loading data from eeprom...");
    String loaddedSettings = loadFromEeprom();
    if (loaddedSettings.length() == 0) {
        ESP_LOGI(SETTINGS_MANAGER_TAG, "Settings empty!");
        return;
    }
    _settings->jload(loaddedSettings);
}

void SettingsManager::clear() {
    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
            EEPROM.write(i, 0);
        }
        ESP_LOGI("EEprom clear");
    } else {
        ESP_LOGI("Failed to open EEPROM");
    }
}

String SettingsManager::loadFromEeprom() {
    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        uint8_t size = EEPROM.read(0);
        if (size == 0) {
            return "";
        }

        char data[size + 1];
        int val = 0;
        for (int i = 0; i < size; i++) {
            data[i] = EEPROM.read(i + 1);
        }

        EEPROM.commit();
        ESP_LOGI(SETTINGS_MANAGER_TAG, "Loaded from eeprom: %s", data);
        return data;
    } else {
        ESP_LOGI("Failed to open EEPROM");
        return "";
    }
}

void SettingsManager::saveSettings() {
    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        uint8_t size = _settings->jsize();
        String data = _settings->json();
        ESP_LOGI(SETTINGS_MANAGER_TAG, "Saving settings: %s", data.c_str());

        EEPROM.write(0, size);
        for (int i = 0; i < size; i++) {
            EEPROM.write(i + 1, data.charAt(i));
        }

        EEPROM.commit();
        ESP_LOGI(SETTINGS_MANAGER_TAG, "Settings saved");
    } else {
        ESP_LOGI("Failed to open EEPROM");
    }
}

void SettingsManager::removeSetting(String name) {
    if (name == SSID_SETTING || name == PASSWORD_SETTING) {
        return;
    }
    _settings->remove(name.c_str());
}

String SettingsManager::getSetting(String name) {
    ESP_LOGI("*", "%s %s", name, _settings->search(name));
    return _settings->search(name);
}

String SettingsManager::getJson() {
    return _settings->json();
}

void SettingsManager::mergeSettings(Dictionary dict) {
    _settings->merge(dict);
}