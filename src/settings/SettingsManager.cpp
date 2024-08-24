#include "settings/SettingsManager.h"
#include "SmartThing.h"

#include <EEPROM.h>

#define GROUP_CONFIG "cg"
#define GROUP_WIFI "wf"
#define DEVICE_NAME "dn"
#define GROUP_HOOKS "cb"

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


SettingsManager STSettings;

SettingsManager::SettingsManager() {}

SettingsManager::~SettingsManager() { _settings.garbageCollect(); }

void SettingsManager::loadSettings() {
  LOGGER.info(SETTINGS_MANAGER_TAG, "Loading data from eeprom...");
  String loaddedSettings = loadFromEeprom();
  if (loaddedSettings.length() == 0) {
    LOGGER.warning(SETTINGS_MANAGER_TAG, "Settings empty! Adding default");
    addDefaultSettings();
  } else {
    deserializeJson(_settings, loaddedSettings);
  }
  _loaded = true;
}

void SettingsManager::addDefaultSettings() {
  #ifdef ARDUINO_ARCH_ESP32
  _settings[DEVICE_NAME] = ESP.getChipModel();
  #endif
  #ifdef ARDUINO_ARCH_ESP8266
  _settings[DEVICE_NAME] = "SMT_DEV";
  #endif
  save();
}

void SettingsManager::clear() {
  if (eepromBegin(EEPROM_LOAD_SIZE)) {
    for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
      EEPROM.write(i, 0);
    }
    LOGGER.warning(SETTINGS_MANAGER_TAG, "EEPROM clear");
  } else {
    LOGGER.error(SETTINGS_MANAGER_TAG, "Failed to open EEPROM");
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
      LOGGER.error(SETTINGS_MANAGER_TAG,
                   "Settings string not completed. Missing \\n ?");
      // LOGGER.error(SETTINGS_MANAGER_TAG, "%s", data.c_str());
      return "";
    }

    data += "}";

    // LOGGER.debug(SETTINGS_MANAGER_TAG, "Loaded from eeprom: %s [%u]",
    //              data.c_str(), data.length());
    LOGGER.debug(SETTINGS_MANAGER_TAG, "Loaded from eeprom data length=%u", data.length());
    return data;
  } else {
    LOGGER.error(SETTINGS_MANAGER_TAG, "Failed to open EEPROM");
    return "";
  }
}

void SettingsManager::removeIfEmpty(const char* group) {
  if (_settings[group].size() == 0) {
    _settings.remove(group);
    LOGGER.debug(SETTINGS_MANAGER_TAG, "Removed group %s from settings - it's empty", group);
  }
}

bool SettingsManager::save() {
  LOGGER.info(SETTINGS_MANAGER_TAG, "Saving settings");
  removeIfEmpty(GROUP_WIFI);
  removeIfEmpty(GROUP_CONFIG);
  removeIfEmpty(GROUP_HOOKS);
  _settings.garbageCollect();

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
    LOGGER.error(SETTINGS_MANAGER_TAG,
                 "Save failed, data are too long! Expected less then %d, got %d",
                 EEPROM_LOAD_SIZE, data.length());
    return false;
  }

  if (eepromBegin(EEPROM_LOAD_SIZE)) {
    LOGGER.debug(SETTINGS_MANAGER_TAG, "Writing data to EEPROM (length [%u])",
                 data.length());
    #if LOGGER_TYPE == SERIAL_LOGGER
    Serial.print(data.c_str());
    #endif
    for (unsigned int i = 0; i < data.length(); i++) {
      EEPROM.write(i, data.charAt(i));
    }
    EEPROM.commit();
    LOGGER.info(SETTINGS_MANAGER_TAG, "Settings saved");
    return true;
  } else {
    LOGGER.error(SETTINGS_MANAGER_TAG, "Save failed, can't open EEPROM");
    return false;
  }
}

void SettingsManager::removeSetting(const char* name) {
  if (strcmp(name, SSID_SETTING) == 0  || strcmp(name, PASSWORD_SETTING) == 0  || strcmp(name, GROUP_WIFI) == 0 ) {
    LOGGER.warning(SETTINGS_MANAGER_TAG,
                   "You can't remove Wifi credits with this function! Use "
                   "dropWifiCredits insted.");
    return;
  }
  _settings.remove(name);
  _settings.garbageCollect();
}

void SettingsManager::dropAll() {
  _settings.clear();
  clear();
}

void SettingsManager::dropWifiCredits() { _settings.remove(GROUP_WIFI); }

JsonObject SettingsManager::getOrCreateObject(const char* name) {
  if (_settings.containsKey(name)) {
    return _settings[name];
  }
  LOGGER.debug(SETTINGS_MANAGER_TAG, "Creating new nested object %s", name);
  return _settings.createNestedObject(name);
}

JsonObject SettingsManager::getConfig() {
  return getOrCreateObject(GROUP_CONFIG);
}

void SettingsManager::dropConfig() {
  if (_settings.containsKey(GROUP_CONFIG)) {
    _settings.remove(GROUP_CONFIG);
    LOGGER.warning(SETTINGS_MANAGER_TAG, "All config values were removed!");
  } else {
    LOGGER.debug(SETTINGS_MANAGER_TAG, "Config settings not exists");
  }
}

JsonObject SettingsManager::getWiFi() { return getOrCreateObject(GROUP_WIFI); }

void SettingsManager::setDeviceName(const char* name) {
  _settings[DEVICE_NAME] = name;
}

const String SettingsManager::getDeviceName() {
  if (_settings.containsKey(DEVICE_NAME)) {
    return _settings[DEVICE_NAME];
  }
  return "";
}

void SettingsManager::setHooks(JsonArray doc) {
  _settings[GROUP_HOOKS] = doc;
}

JsonArray SettingsManager::getHooks() {
  if (_settings.containsKey(GROUP_HOOKS)) {
    return _settings[GROUP_HOOKS];
  }
  return _settings.createNestedArray(GROUP_HOOKS);
}

void SettingsManager::dropAllHooks() {
  _settings.remove(GROUP_HOOKS);
  _settings.garbageCollect();
}

const DynamicJsonDocument SettingsManager::exportSettings() {
  DynamicJsonDocument doc(JSON_SETTINGS_DOC_SIZE);
  doc[GROUP_CONFIG] = getConfig();
  doc[GROUP_HOOKS] = getHooks();
  doc[DEVICE_NAME] = getDeviceName();
  return doc;
}

bool SettingsManager::importSettings(DynamicJsonDocument doc) {
  if (doc.size() == 0) {
    LOGGER.info(SETTINGS_MANAGER_TAG, "Empty settings json!");
    return false;
  }
  bool res = true;
  String old;
  serializeJson(_settings, old);
  LOGGER.debug(SETTINGS_MANAGER_TAG, "Old settings save length=%u", old.length());

  String name = doc[DEVICE_NAME];
  if (!name.isEmpty()) {
    if (name.length() > DEVICE_NAME_LENGTH_MAX) {
      res = false;
      LOGGER.error(SETTINGS_MANAGER_TAG, "Device name is too long! Max length: %d", DEVICE_NAME_LENGTH_MAX);
    } else {
      LOGGER.info(SETTINGS_MANAGER_TAG, "New name: %s", name.c_str());
      _settings[DEVICE_NAME] = name;
    }
  }
  
  if (doc[GROUP_CONFIG].size() > 0 && !doc[GROUP_CONFIG].is<JsonObject>()) {
    LOGGER.error(SETTINGS_MANAGER_TAG, "Expected %s to be JsonObject!", GROUP_CONFIG);
    res = false;
  } else {
    _settings[GROUP_CONFIG] = doc[GROUP_CONFIG];
  }
  if (doc[GROUP_HOOKS].size() > 0 && !doc[GROUP_HOOKS].is<JsonArray>()) {
    LOGGER.error(SETTINGS_MANAGER_TAG, "Expected %s to be JsonArray!", GROUP_HOOKS);
    res = false;
  } else {
    _settings[GROUP_HOOKS] = doc[GROUP_HOOKS];
  }

  if (res) {
    res = save();
  }

  if (!res) {
    LOGGER.info(SETTINGS_MANAGER_TAG, "Import failed, rollback old settings");
    deserializeJson(_settings, old);
    save();
  }

  return res;
}

const DynamicJsonDocument SettingsManager::getAllSettings() {
  return _settings;
}