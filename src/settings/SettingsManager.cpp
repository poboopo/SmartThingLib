#include "settings/SettingsManager.h"
#include "SmartThing.h"

#include <EEPROM.h>

#ifdef ARDUINO_ARCH_ESP32
bool eepromBegin(size_t size) {
  return EEPROM.begin(size);
}
#endif

#ifdef ARDUINO_ARCH_ESP8266
bool eepromBegin(size_t size) {
  EEPROM.begin(size);
  return true;
}
#endif

static const char * SETTINGS_MANAGER_TAG = "settings_manager";

SettingsManager STSettings;

SettingsManager::SettingsManager() {}

SettingsManager::~SettingsManager() { _settings.clear(); }

void SettingsManager::loadSettings() {
  ST_LOG_INFO(SETTINGS_MANAGER_TAG, "Loading data from eeprom...");
  String loaddedSettings = loadFromEeprom();
  if (loaddedSettings.length() == 0) {
    ST_LOG_WARNING(SETTINGS_MANAGER_TAG, "Settings empty! Adding default");
  } else {
    deserializeJson(_settings, loaddedSettings);
  }
  _loaded = true;
}

void SettingsManager::clear() {
  if (eepromBegin(EEPROM_LOAD_SIZE)) {
    for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
      EEPROM.write(i, 0);
    }
    ST_LOG_WARNING(SETTINGS_MANAGER_TAG, "EEPROM clear");
  } else {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Failed to open EEPROM");
  }
}

String SettingsManager::loadFromEeprom() {
  if (eepromBegin(EEPROM_LOAD_SIZE)) {
    String data = "{";
    uint8_t val;
    bool completed = false;
    for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
      val = EEPROM.read(i);
      if (isAscii(val)) {
        if (val == '\n') {
          completed = true;
          break;
        }
        data += (char)val;
      }
    }

    if (!completed) {
      ST_LOG_ERROR(SETTINGS_MANAGER_TAG,
                   "Settings string not completed. Missing \\n ?");
      // ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "%s", data.c_str());
      return "";
    }

    data += "}";

    // ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Loaded from eeprom: %s [%u]",
    //              data.c_str(), data.length());
    ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Loaded from eeprom data length=%u", data.length());
    return data;
  } else {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Failed to open EEPROM");
    return "";
  }
}

void SettingsManager::removeIfEmpty(const char* group) {
  if (_settings[group].isNull() || _settings[group].size() == 0) {
    _settings.remove(group);
    ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Removed group %s from settings - it's empty", group);
  }
}

bool SettingsManager::save() {
  ST_LOG_INFO(SETTINGS_MANAGER_TAG, "Saving settings");
  removeIfEmpty(GROUP_WIFI);
  removeIfEmpty(GROUP_CONFIG);
  removeIfEmpty(GROUP_HOOKS);
  removeIfEmpty(GROUP_ACTIONS);
  // _settings.garbageCollect();

  String data;
  serializeJson(_settings, data);

  if (data == "null") {
    data.clear();
  } else {
    data.remove(0, 1);
    data.remove(data.length() - 1);
    data += "\n";
  }

  if (data.length() > EEPROM_LOAD_SIZE) {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG,
                 "Save failed, data are too long! Expected less then %d, got %d",
                 EEPROM_LOAD_SIZE, data.length());
    return false;
  }

  if (eepromBegin(EEPROM_LOAD_SIZE)) {
    ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Writing data to EEPROM (length [%u])",
                 data.length());
    #if LOGGER_TYPE == SERIAL_LOGGER
    Serial.print(data.c_str());
    #endif
    for (unsigned int i = 0; i < data.length(); i++) {
      EEPROM.write(i, data.charAt(i));
    }
    EEPROM.commit();
    ST_LOG_INFO(SETTINGS_MANAGER_TAG, "Settings saved");
    return true;
  } else {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Save failed, can't open EEPROM");
    return false;
  }
}

void SettingsManager::removeSetting(const char* name) {
  if (strcmp(name, SSID_SETTING) == 0  || strcmp(name, PASSWORD_SETTING) == 0  || strcmp(name, GROUP_WIFI) == 0 ) {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG,
                   "You can't remove Wifi credits with this function! Use "
                   "dropWifiCredits insted.");
    return;
  }
  _settings.remove(name);
  // _settings.garbageCollect();
}

void SettingsManager::wipeAll() {
  _settings.clear();
  clear();
}

void SettingsManager::dropWifiCredits() { _settings.remove(GROUP_WIFI); }

JsonObject SettingsManager::getOrCreateObject(const char* name) {
  if (_settings.containsKey(name)) {
    return _settings[name];
  }
  ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Creating new nested object %s", name);
  return _settings[name].to<JsonObject>();
}

JsonObject SettingsManager::getConfig() {
  return getOrCreateObject(GROUP_CONFIG);
}

void SettingsManager::dropConfig() {
  if (_settings.containsKey(GROUP_CONFIG)) {
    _settings.remove(GROUP_CONFIG);
    ST_LOG_WARNING(SETTINGS_MANAGER_TAG, "All config values were removed!");
  } else {
    ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Config settings not exists");
  }
}

JsonObject SettingsManager::getWiFi() { return getOrCreateObject(GROUP_WIFI); }

void SettingsManager::setDeviceName(const char * name) {
  if (strlen(name) == 0) {
    _settings.remove(DEVICE_NAME);
  } else {
    _settings[DEVICE_NAME] = name;
  }
}

const char * SettingsManager::getDeviceName() {
  if (_settings.containsKey(DEVICE_NAME)) {
    return _settings[DEVICE_NAME];
  }
  return ST_DEFAULT_NAME;
}

void SettingsManager::setHooks(JsonDocument doc) {
  _settings[GROUP_HOOKS] = doc;
}

JsonDocument SettingsManager::getHooks() {
  return _settings[GROUP_HOOKS];
}

void SettingsManager::dropAllHooks() {
  _settings.remove(GROUP_HOOKS);
}

void SettingsManager::setActionsConfig(JsonDocument doc) {
  _settings[GROUP_ACTIONS] = doc;
}

JsonDocument SettingsManager::getActionsConfig() {
  return getOrCreateObject(GROUP_ACTIONS);
}

const JsonDocument SettingsManager::exportSettings() {
  JsonDocument doc;
  doc[GROUP_CONFIG] = getConfig();
  doc[GROUP_HOOKS] = getHooks();
  doc[DEVICE_NAME] = getDeviceName();
  doc[GROUP_ACTIONS] = getActionsConfig();
  return doc;
}

bool SettingsManager::importSettings(JsonDocument doc) {
  if (doc.size() == 0) {
    ST_LOG_INFO(SETTINGS_MANAGER_TAG, "Empty settings json!");
    return false;
  }
  bool res = true;
  String old;
  serializeJson(_settings, old);
  ST_LOG_DEBUG(SETTINGS_MANAGER_TAG, "Old settings save length=%u", old.length());

  String name = doc[DEVICE_NAME];
  if (!name.isEmpty()) {
    if (name.length() > DEVICE_NAME_LENGTH_MAX) {
      res = false;
      ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Device name is too long! Max length: %d", DEVICE_NAME_LENGTH_MAX);
    } else {
      ST_LOG_INFO(SETTINGS_MANAGER_TAG, "New name: %s", name.c_str());
      _settings[DEVICE_NAME] = name;
    }
  }
  
  if (doc[GROUP_CONFIG].size() > 0 && !doc[GROUP_CONFIG].is<JsonObject>()) {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Expected %s to be JsonObject!", GROUP_CONFIG);
    res = false;
  } else {
    _settings[GROUP_CONFIG] = doc[GROUP_CONFIG];
  }
  if (doc[GROUP_HOOKS].size() > 0 && !doc[GROUP_HOOKS].is<JsonArray>()) {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Expected %s to be JsonArray!", GROUP_HOOKS);
    res = false;
  } else {
    _settings[GROUP_HOOKS] = doc[GROUP_HOOKS];
  }
  if (doc[GROUP_ACTIONS].size() > 0 && !doc[GROUP_ACTIONS].is<JsonObject>()) {
    ST_LOG_ERROR(SETTINGS_MANAGER_TAG, "Expected %s to be JsonObject!", GROUP_ACTIONS);
    res = false;
  } else {
    _settings[GROUP_ACTIONS] = doc[GROUP_ACTIONS];
  }

  if (res) {
    res = save();
  }

  if (!res) {
    ST_LOG_INFO(SETTINGS_MANAGER_TAG, "Import failed, rollback old settings");
    deserializeJson(_settings, old);
    save();
  }

  return res;
}

const JsonDocument SettingsManager::getAllSettings() {
  return _settings;
}