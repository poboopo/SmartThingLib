#include <utils/SettingsManager.h>
#include <EEPROM.h>

#define EEPROM_LOAD_SIZE 1024

bool SettingsManager::_loaded = false;
StaticJsonDocument<JSON_DOC_SIZE> SettingsManager::_settings = StaticJsonDocument<JSON_DOC_SIZE>();

SettingsManager::SettingsManager() {
}

SettingsManager::~SettingsManager() {
    _settings.garbageCollect();
}

void SettingsManager::loadSettings() {
    BetterLogger::log(SETTINGS_MANAGER_TAG, "Loading data from eeprom...");
    const char * loaddedSettings = loadFromEeprom();
    if (strlen(loaddedSettings) == 0) {
        BetterLogger::log(SETTINGS_MANAGER_TAG, "Settings empty!");
        return;
    }
    deserializeJson(_settings, loaddedSettings);
    _loaded = true;
}

void SettingsManager::clear() {
    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
        for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
            EEPROM.write(i, 0);
        }
        BetterLogger::log("EEprom clear");
    } else {
        BetterLogger::log("Failed to open EEPROM");
    }
}

const char * SettingsManager::loadFromEeprom() {
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
            BetterLogger::log("Settings string not completed. Missing \\n ?");
            BetterLogger::log("%s", data.c_str());
            return "";
        }

        data += "}";

        BetterLogger::log(SETTINGS_MANAGER_TAG, "Loaded from eeprom: %s [%u]", data.c_str(), data.length());
        return data.c_str();
    } else {
        BetterLogger::log("Failed to open EEPROM");
        return "";
    }
}

void SettingsManager::saveSettings() {
    String data;
    serializeJson(_settings, data);
    BetterLogger::log(SETTINGS_MANAGER_TAG, "Parsed json: %s", data.c_str());

    // Убираем скобки, что не тратить драгоценное место EEPROM
    data.remove(0, 1);
    data.remove(data.length() - 1);
    data += "\n";

    if (data.length() > EEPROM_LOAD_SIZE) {
       BetterLogger::log(SETTINGS_MANAGER_TAG, "Settings are too long! Expected less then %d, got %d", EEPROM_LOAD_SIZE, data.length());
        return;
    }

    if (EEPROM.begin(EEPROM_LOAD_SIZE)) {
       BetterLogger::log(SETTINGS_MANAGER_TAG, "Saving settings (length [%u]): %s", data.length(), data.c_str());
        for (int i = 0; i < data.length(); i++) {
            EEPROM.write(i, data.charAt(i));
        }

        EEPROM.commit();
       BetterLogger::log(SETTINGS_MANAGER_TAG, "Settings saved");
    } else {
       BetterLogger::log("Failed to open EEPROM");
    }
}

void SettingsManager::removeSetting(String name) {
    if (name == SSID_SETTING || name == PASSWORD_SETTING || name == GROUP_WIFI) {
       BetterLogger::log(SETTINGS_MANAGER_TAG, "U can't remove Wifi credits with this function! Use dropWifiCredits insted.");
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

const String SettingsManager::getSettingString(String name) {
    if (_settings.containsKey(name)) {
        return _settings[name];
    }
    return "";
}

const String SettingsManager::getSettingString(String groupName, String name) {
    if (_settings.containsKey(groupName)) {
        return _settings[groupName][name];
    }
    return "";
}

const int SettingsManager::getSettingInteger(String name) {
    if (_settings.containsKey(name)) {
        return _settings[name];
    }
    return 0;
}

const int SettingsManager::getSettingInteger(String groupName, String name) {
    if (_settings.containsKey(groupName)) {
        return _settings[groupName][name];
    }
    return 0;
}

const JsonObject SettingsManager::getSettings() {
    return _settings.to<JsonObject>();
}

const JsonObject SettingsManager::getSettings(String groupName) {
    JsonObject settings = _settings[groupName];
    return settings;
}

const String SettingsManager::getJson() {
    String json;
    serializeJson(_settings, json);
    return json;
}

const String SettingsManager::getJson(String groupName) {
    String json;
    serializeJson(getSettings(groupName), json);
    return json;
}