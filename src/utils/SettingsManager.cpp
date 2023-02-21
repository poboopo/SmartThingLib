#include <utils/SettingsManager.h>
#include <EEPROM.h>

#define EEPROM_LOAD_SIZE 2048

SettingsManager::SettingsManager() {
}

SettingsManager::~SettingsManager() {
    _settings.garbageCollect();
}

void SettingsManager::addLogger(BetterLogger * logger) {
    _logger = logger;
}

void SettingsManager::loadSettings() {
    if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Loading data from eeprom...");
    String loaddedSettings = loadFromEeprom();
    if (loaddedSettings.length() == 0) {
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Settings empty!");
        return;
    }
    deserializeJson(_settings, loaddedSettings);
}

void SettingsManager::clear() {
    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
            EEPROM.write(i, 0);
        }
        if (_logger != NULL) _logger->log("EEprom clear");
    } else {
        if (_logger != NULL) _logger->log("Failed to open EEPROM");
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
            val = EEPROM.read(i + 1);
            data[i] = isAscii(val) ? val : ' ';
        }

        EEPROM.commit();
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Loaded from eeprom: %s [%u]", data, size);
        return data;
    } else {
        if (_logger != NULL) _logger->log("Failed to open EEPROM");
        return "";
    }
}

void SettingsManager::saveSettings() {
    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        String data;
        serializeJson(_settings, data);
        uint16_t size = data.length();
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Saving settings: %s [%u]", data.c_str(), size);
        EEPROM.write(0, size);
        for (int i = 0; i < size; i++) {
            EEPROM.write(i + 1, data.charAt(i));
        }

        EEPROM.commit();
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Settings saved");
    } else {
        if (_logger != NULL) _logger->log("Failed to open EEPROM");
    }
}

void SettingsManager::removeSetting(String name) {
    if (name == SSID_SETTING || name == PASSWORD_SETTING || name == GROUP_WIFI) {
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "U can't remove Wifi credits with this function! Use dropWifiCredits insted.");
        return;
    }
    _settings.remove(name.c_str());
}

void SettingsManager::dropWifiCredits() {
    _settings.remove(GROUP_WIFI);
    saveSettings();
}

void SettingsManager::putSetting(String groupName, JsonObject jsonObject) {
    for (JsonPair pair: jsonObject) {
        _settings[groupName][pair.key().c_str()] = pair.value();
    }
}

void SettingsManager::putSetting(String groupName, String name, JsonVariant value) {
    _settings[groupName][name] = value;
}

void SettingsManager::putSetting(String groupName, String name, String value) {
    _settings[groupName][name] = value;
}

void SettingsManager::putSetting(String name, String value) {
    _settings[name] = value;
}

void SettingsManager::putSetting(String groupName, String name, int value) {
    _settings[groupName][name] = value;
}

void SettingsManager::putSetting(String name, int value) {
    _settings[name] = value;
}

String SettingsManager::getSettingString(String name) {
    if (_settings.containsKey(name)) {
        return _settings[name];
    }
    return "";
}

String SettingsManager::getSettingString(String groupName, String name) {
    if (_settings.containsKey(groupName)) {
        return _settings[groupName][name];
    }
    return "";
}

int SettingsManager::getSettingInteger(String name) {
    if (_settings.containsKey(name)) {
        return _settings[name];
    }
    return 0;
}

int SettingsManager::getSettingInteger(String groupName, String name) {
    if (_settings.containsKey(groupName)) {
        return _settings[groupName][name];
    }
    return 0;
}

JsonObject SettingsManager::getSettings() {
    return _settings.to<JsonObject>();
}

JsonObject SettingsManager::getSettings(String groupName) {
    JsonObject settings = _settings[groupName];
    return settings;
}

String SettingsManager::getJson() {
    String json;
    serializeJson(_settings, json);
    return json;
}

String SettingsManager::getJson(String groupName) {
    String json;
    serializeJson(getSettings(groupName), json);
    return json;
}