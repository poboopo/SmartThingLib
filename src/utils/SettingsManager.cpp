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
        String data = "{";
        uint8_t val;
        bool completed = false;
        for (int i = 0; i < EEPROM_LOAD_SIZE; i++){
            val = EEPROM.read(i);
            if (isAscii(val)) {
                if (val == '\n') {
                    completed = true;
                    break;
                }
                data += (char) val;
            }
        }
        EEPROM.commit();

        if (!completed) {
            if (_logger != NULL) {
                _logger->log("Settings string not completed. Missing \\n ?");
                _logger->log("%s", data.c_str());
            } 
            return "";
        }

        data += "}";

        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Loaded from eeprom: %s [%u]", data.c_str(), data.length());
        return data;
    } else {
        if (_logger != NULL) _logger->log("Failed to open EEPROM");
        return "";
    }
}

void SettingsManager::saveSettings() {
    String data;
    serializeJson(_settings, data);
    if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Parsed json: %s", data.c_str());

    // Убираем скобки, что не тратить драгоценное место EEPROM
    data.remove(0, 1);
    data.remove(data.length() - 1);
    data += "\n";

    if (data.length() > EEPROM_LOAD_SIZE) {
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Settings are too long! Expected less then %d, got %d", EEPROM_LOAD_SIZE, data.length());
        return;
    }

    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        if (_logger != NULL) _logger->log(SETTINGS_MANAGER_TAG, "Saving settings (length [%u]): %s", data.length(), data.c_str());
        for (int i = 0; i < data.length(); i++) {
            EEPROM.write(i, data.charAt(i));
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

void SettingsManager::dropAll() {
    _settings.clear();
}

void SettingsManager::dropWifiCredits() {
    _settings.remove(GROUP_WIFI);
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