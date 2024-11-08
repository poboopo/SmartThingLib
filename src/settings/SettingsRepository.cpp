#include "settings/SettingsRepository.h"
#include "SmartThing.h"

#include <EEPROM.h>

// [offsets][name][wifi][config][hooks][actions]
#define SETTINGS_TEMPLATE "%03d%03d%03d%03d%03d%s%s%s%s%s"
#define LENGTH_PARTITION_SIZE 3
#define DATA_OFFSET 15
#define DEFAULT_NAME_LENGTH 10

#define GROUP_CONFIG "configuration"
#define GROUP_WIFI "wifi"
#define GROUP_HOOKS "hooks"
#define GROUP_ACTIONS "actions"
#define GROUP_NAME "name"

#ifdef ARDUINO_ARCH_ESP32
bool eepromBegin() {
  return EEPROM.begin(EEPROM_LOAD_SIZE);
}
#endif

#ifdef ARDUINO_ARCH_ESP8266
bool eepromBegin() {
  EEPROM.begin(EEPROM_LOAD_SIZE);
  return true;
}
#endif

enum DataIndex {
  NAME_INDEX,
  WIFI_INDEX,
  CONFIG_INDEX,
  HOOKS_INDEX,
  ACTIONS_INDEX,
  FIRST_INDEX = NAME_INDEX,
  LAST_INDEX = ACTIONS_INDEX
};

static const char * SETTINGS_MANAGER_TAG = "settings_manager";
static const char * EEPROM_OPEN_ERROR = "Failed to open EEPROM";

SettingsRepositoryClass SettingsRepository;

SettingsRepositoryClass::SettingsRepositoryClass() {}
SettingsRepositoryClass::~SettingsRepositoryClass() {}

void SettingsRepositoryClass::clear() {
  if (eepromBegin()) {
    for (int i = 0; i < EEPROM_LOAD_SIZE; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    EEPROM.end();
    st_log_warning(SETTINGS_MANAGER_TAG, "EEPROM clear");
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, EEPROM_OPEN_ERROR);
  }
}

void SettingsRepositoryClass::read(uint16_t address, char * buff, uint16_t length) {
  for (uint16_t i = 0; i < length; i++) {
    buff[i] = (char) EEPROM.read(address + i);
  }
  buff[length] = 0;
}

void SettingsRepositoryClass::write(uint16_t address, const char * buff, uint16_t length) {
  for (uint16_t i = 0; i < length; i++) {
    EEPROM.write(address + i, buff[i]);
  }
}

int SettingsRepositoryClass::getLength(uint8_t index) {
  if (index < FIRST_INDEX || index > LAST_INDEX) {
    return -1;
  }
  char * buff = (char *) malloc(LENGTH_PARTITION_SIZE + 1);
  read(index * LENGTH_PARTITION_SIZE, buff, LENGTH_PARTITION_SIZE);
  int result = atoi(buff);
  free(buff);
  return result;
}

int SettingsRepositoryClass::writeLength(uint8_t index, int length) {
  if (index < FIRST_INDEX || index > LAST_INDEX) {
    return -1;
  }
  char * buff = (char *) malloc(LENGTH_PARTITION_SIZE + 1);
  sprintf(buff, "%03d", length);

  write(index * LENGTH_PARTITION_SIZE, buff, LENGTH_PARTITION_SIZE);

  free(buff);
  return length;
}

String SettingsRepositoryClass::readData(uint8_t index, const char * defaultValue) {
  if (index < FIRST_INDEX || index > LAST_INDEX) {
    return defaultValue;
  }
  if (eepromBegin()) {
    int targetLength = getLength(index);
    if (targetLength == 0) {
      EEPROM.end();
      return defaultValue;
    }

    int offset = DATA_OFFSET;
    for (int i = 0; i < index; i++) {
      offset += getLength(i);
    }

    char * buff = (char *) malloc(targetLength + 1);
    read(offset, buff, targetLength);

    String result = buff;
    free(buff);

    EEPROM.end();
    return result;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, EEPROM_OPEN_ERROR);
    return defaultValue;
  }
}

int SettingsRepositoryClass::writeData(uint8_t index, const char * data) {
  if (index < FIRST_INDEX || index > LAST_INDEX || data == nullptr) {
    return -1;
  }

  if (eepromBegin()) {
    int tmp = 0, offset = DATA_OFFSET, targetLength = 0, tailLength = 0;
    targetLength = getLength(index);
    if (targetLength < 0) {
      return -1;
    }

    for (int i = 0; i < index; i++) {
      tmp = getLength(i);
      if (tmp < 0) {
        return -1;
      }
      offset += tmp;
    }

    char * oldData = (char *) malloc(targetLength + 1);
    read(offset, oldData, targetLength);
    int cmp = strcmp(oldData, data);
    free(oldData);
    if (cmp == 0) {
      st_log_debug(SETTINGS_MANAGER_TAG, "Old data equals new, not writing");
      return targetLength;
    }

    for (int i = index + 1; i <= LAST_INDEX; i++) {
      tmp = getLength(i);
      if (tmp < 0) {
        return -1;
      }
      tailLength += tmp;
    }

    char * buffTail = (char *) malloc(tailLength + 1);
    read(offset + targetLength, buffTail, tailLength);
    
    int dataLen = strlen(data);

    write(offset, data, dataLen);
    write(offset + dataLen, buffTail, strlen(buffTail));
    writeLength(index, dataLen);

    EEPROM.commit();
    EEPROM.end();

    free(buffTail);
    return dataLen;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, EEPROM_OPEN_ERROR);
    return -1;
  }
}

JsonDocument SettingsRepositoryClass::stringToObject(String& data) {
  JsonDocument doc;
  doc.to<JsonObject>();
  if (!data.isEmpty()) {
    String buff, key;
    char tmp;
    bool escaped = false, buildKey = true;
    for (uint8_t i = 0; i < data.length(); i++) {
      tmp = data.charAt(i);

      if (!escaped && tmp == '|') {
        escaped = true;
      } else if (!escaped && tmp == ':') {
        if (buildKey) {
          key = buff;
          buff.clear();
          buildKey = false;
        } else {
          doc[key] = buff;
          key.clear();
          buff.clear();
          buildKey = true;
        }
      } else {
        if (escaped && tmp != ':') {
          buff += '|';
        }
        buff += tmp;
        escaped = false;
      }
    }
    doc[key] = buff;
  }
  return doc;
}

String SettingsRepositoryClass::objectToString(JsonDocument doc) {
  String res = "";
  JsonObject root = doc.as<JsonObject>();
  for (JsonPair pair: root) {
    if (!pair.value().isNull()) {
      String value = pair.value().as<String>();
      if (value.isEmpty()) {
        continue;
      }
      String key = pair.key().c_str();
      key.replace(":", "|:");
      value.replace(":", "|:");

      res += key;
      res += ":";
      res += value;
      res += ":";
    }
  }
  res.remove(res.length() - 1);
  return res;
}

String SettingsRepositoryClass::getName() {
  return readData(NAME_INDEX, ST_DEFAULT_NAME);
}

bool SettingsRepositoryClass::setName(String name) {
  if (name.length() > DEVICE_NAME_LENGTH_MAX) {
    st_log_error(SETTINGS_MANAGER_TAG, "Name is too big! Max name length=%d", DEVICE_NAME_LENGTH_MAX);
    return false;
  }
  
  if (writeData(NAME_INDEX, name.c_str()) >= 0) {
    return true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "Name update failed");
    return false;
  }
}

WiFiConfig SettingsRepositoryClass::getWiFi() {
  WiFiConfig settings;

  String settingsStr = readData(WIFI_INDEX);
  if (settingsStr.isEmpty()) {
    st_log_warning(SETTINGS_MANAGER_TAG, "WiFi config empty");
    return settings;
  }

  String buff;
  char tmp;
  bool ssid = true, escaped = false;
  for (uint8_t i = 0; i < settingsStr.length(); i++) {
    tmp = settingsStr.charAt(i);
    if (!escaped && tmp == '|') {
      escaped = true;
    } else if (!escaped && tmp == ':') {
      if (ssid) {
        settings.ssid = buff;
        ssid = false;
      } else {
        settings.password = buff;
      }
      buff.clear();
    } else {
      if (escaped && tmp != ':') {
        buff += '|';
      }
      buff += tmp;
      escaped = false;
    }
  }
  settings.mode = buff.toInt();
  return settings;
}

bool SettingsRepositoryClass::setWiFi(WiFiConfig settings) {
  settings.ssid.replace(":", "|:");
  settings.password.replace(":", "|:");

  char * buff = (char *) malloc(settings.ssid.length() + settings.password.length() + 4);
  sprintf(
    buff,
    "%s:%s:%d",
    settings.ssid.c_str(),
    settings.password.c_str(),
    settings.mode
  );

  bool res = false;
  if (writeData(WIFI_INDEX, buff)) {
    st_log_debug(SETTINGS_MANAGER_TAG, "WiFi config updated: %s", buff);
    res = true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "WiFi config update failed");
  }
  free(buff);
  return res;
}

bool SettingsRepositoryClass::dropWiFi() {
  bool res = false;
  if (writeData(WIFI_INDEX, "") == 0) {
    st_log_warning(SETTINGS_MANAGER_TAG, "WiFi config droped");
    res = true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "WiFi conig drop failed");
  }
  return res;
}

JsonDocument SettingsRepositoryClass::getConfig() {
  if (_configEntriesList.size() == 0) {
    JsonDocument doc;
    return doc;
  }

  String data = readData(CONFIG_INDEX);
  return stringToObject(data);
}

bool SettingsRepositoryClass::setConfig(JsonDocument conf) {
  bool res = false;
  // todo validate config entries
  if (_configEntriesList.size() == 0) {
    return res;
  }

  String data = objectToString(conf);
  if (writeData(CONFIG_INDEX, data.c_str()) >= 0) {
    res = true;
    st_log_debug(SETTINGS_MANAGER_TAG, "Configuration updated");
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "Configuration update failed");
  }
  return res;
}

bool SettingsRepositoryClass::dropConfig() {
  bool res = false;
  if (writeData(CONFIG_INDEX, "") == 0) {
    st_log_warning(SETTINGS_MANAGER_TAG, "Config droped");
    res = true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "Configuration drop failed");
  }
  return res;
}

JsonDocument SettingsRepositoryClass::getConfigInfoJson() {
  return _configEntriesList.toJson();
}

bool SettingsRepositoryClass::addConfigEntry(const char* name, const char* caption, ConfigEntryType type) {
  return _configEntriesList.add(name, caption, type);
}

#if ENABLE_HOOKS
bool SettingsRepositoryClass::setHooks(JsonDocument doc) {
  bool res = false;
  String data;
  serializeJson(doc, data);
  if (writeData(HOOKS_INDEX, data.equals("[]") ? "" : data.c_str()) >= 0) {
    st_log_debug(SETTINGS_MANAGER_TAG, "Hooks updated");
    res = true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "Failed to update hooks");
  }
  return res;
}

JsonDocument SettingsRepositoryClass::getHooks() {
  JsonDocument doc;
  String data = readData(HOOKS_INDEX, "[]");
  deserializeJson(doc, data);
  return doc;
}

bool SettingsRepositoryClass::dropHooks() {
  bool res = false;
  if (writeData(CONFIG_INDEX, "")) {
    st_log_debug(SETTINGS_MANAGER_TAG, "Hooks dropped");
    res = true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "Failed to drop hooks");
  }
  return res;
}
#endif

#if ENABLE_ACTIONS_SCHEDULER
bool SettingsRepositoryClass::setActions(JsonDocument conf) {
  String data = objectToString(conf);
  bool res = false;
  if (writeData(ACTIONS_INDEX, data.c_str()) >= 0) {
    st_log_debug(SETTINGS_MANAGER_TAG, "Actions config updated to %s", data.c_str());
    res = true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, "Actions config update failed");
  }
  return res;
}

JsonDocument SettingsRepositoryClass::getActions() {
  String data = readData(ACTIONS_INDEX);
  return stringToObject(data);
}
#endif

String SettingsRepositoryClass::exportSettings() {
  String result = "";
  if (eepromBegin()) {
    uint8_t tmp = 0;
    int actualSize = DATA_OFFSET;
    for (uint8_t i = 0; i <= LAST_INDEX; i++) {
      actualSize += getLength(i);
    }

    char * buff = (char *) malloc(actualSize + 1);
    for (uint16_t i = 0; i < actualSize; i++) {
      tmp = EEPROM.read(i);
      if (tmp == 0) {
        if (i < DATA_OFFSET) {
          tmp = '0';
        } else {
          continue;
        }
      }
      buff[i] = (char) tmp;
    }
    buff[actualSize] = 0;
    EEPROM.end(); 
    result = buff;
    free(buff);
    return result;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, EEPROM_OPEN_ERROR);
    return result;
  }
}

bool SettingsRepositoryClass::importSettings(String dump) {
  if (dump.length() < LENGTH_PARTITION_SIZE) {
    st_log_error(SETTINGS_MANAGER_TAG, "Bad dump - too short");
    return false;
  }

  bool valid = true;
  char tmp;
  for (uint8_t i = 0; i < LENGTH_PARTITION_SIZE; i++) {
    tmp = dump.charAt(i);
    if ('0' > tmp && tmp > '9') {
      valid = false;
      break;
    }
  }
  // todo check if dump contains only ascii?
  if (!valid) {
    st_log_error(SETTINGS_MANAGER_TAG, "Bad dump - worng partitions lengths");
    return false;
  }

  if (eepromBegin()) {
    st_log_warning(SETTINGS_MANAGER_TAG, "Writing dump in eeprom (size=%d)", dump.length());
    st_log_debug(SETTINGS_MANAGER_TAG, "Dump=%s", dump.c_str());

    for (uint16_t i = 0; i < dump.length(); i++) {
      EEPROM.write(i, dump.charAt(i));
    }
    EEPROM.commit();
    EEPROM.end();
    st_log_warning(SETTINGS_MANAGER_TAG, "Dump write finished");
    return true;
  } else {
    st_log_error(SETTINGS_MANAGER_TAG, EEPROM_OPEN_ERROR);
    return false;
  }
}