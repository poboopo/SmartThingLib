#include "config/ConfigManager.h"
#include "logs/BetterLogger.h"
#include "settings/SettingsRepository.h"

const char * const _CONFIG_MANAGER_TAG = "settings_manager";
const char * const _errorConfigEntryNotFound = "Config entry with name %s not found";

ConfigManagerClass ConfigManager;

ConfigManagerClass::ConfigManagerClass() {}
ConfigManagerClass::~ConfigManagerClass() {}


std::list<ConfigEntry*>::iterator ConfigManagerClass::findConfigEntry(const char * name) {
  return std::find_if(_config.begin(), _config.end(), [name](const ConfigEntry * entry) {
    return strcmp(entry->name(), name) == 0;
  });
}

bool ConfigManagerClass::addConfigEntry(const char* name) {
  if (name == nullptr) {
    return false;
  }
  size_t len = strlen(name);
  if (len == 0 || len > MAX_CONFIG_ENTRY_NAME_LENGTH) {
    st_log_warning(_CONFIG_MANAGER_TAG, "Config entry name is too long! Max length = %d", MAX_CONFIG_ENTRY_NAME_LENGTH);
    return false;
  }

  String fixedName = name;
  fixedName.replace(" ", "-");
  fixedName.replace(";", "-");

  auto it = findConfigEntry(fixedName.c_str());
  if (it != _config.end()) {
    st_log_warning(_CONFIG_MANAGER_TAG, "Config entry %s already exists!", fixedName);
    return false;
  }

  _config.push_back(new ConfigEntry(fixedName.c_str()));
  st_log_debug(_CONFIG_MANAGER_TAG, "Added new config entry - %s", fixedName);
  return true;
}

String ConfigManagerClass::getConfigJson() {
  String result = "{";

  for (auto it = _config.begin(); it != _config.end(); ++it) {
    ConfigEntry * current = *it;
    char buff[strlen(current->name()) + strlen(current->value()) + 7];
    sprintf(
      buff,
      "\"%s\":\"%s\"%s",
      current->name(),
      current->value(),
      it == std::prev(_config.end()) ? "" : "," 
    );
    result += String(buff);
  }

  result += "}";
  return result;
}


const char * ConfigManagerClass::getConfigValue(const char * name) {
  if (name == nullptr || strlen(name) == 0) {
    return "";
  }

  auto it = findConfigEntry(name);
  if (it == _config.end()) {
    st_log_warning(_CONFIG_MANAGER_TAG, _errorConfigEntryNotFound, name);
    return "";
  }

  return (*it)->value();
}

bool ConfigManagerClass::setConfigValueWithoutSave(const char * name, const char * value) {
  if (name == nullptr) {
    return false;
  }

  auto it = findConfigEntry(name);
  if (it == _config.end()) {
    st_log_warning(_CONFIG_MANAGER_TAG, _errorConfigEntryNotFound, name);
    return false;
  }


  (*it)->setValue(value);
  return true;
}

bool ConfigManagerClass::setConfigValue(const char * name, const char * value) {
  if (setConfigValueWithoutSave(name, value)) {
    return saveConfig();
  }
  return false;
}

bool ConfigManagerClass::setConfig(JsonDocument conf) {
  if (_config.size() == 0) {
    st_log_error(_CONFIG_MANAGER_TAG, "No config entries were added");
    return false;
  }

  for (auto it = _config.begin(); it != _config.end(); ++it) {
    ConfigEntry * current = *it;

    if (conf[current->name()].is<String>()) {
      String value = conf[current->name()].as<String>();
      current->setValue(value.isEmpty() ? nullptr : value.c_str());
    } else {
      current->setValue(nullptr);
    }
  }
  
  return saveConfig();
}

bool ConfigManagerClass::dropConfig() {
  bool res = false;
  
  if (SettingsRepository.setConfig(emptyString)) {
    for (auto it = _config.begin(); it != _config.end(); ++it) {
      (*it)->setValue(nullptr);
    }

    st_log_warning(_CONFIG_MANAGER_TAG, "Config droped");
    callConfigUpdateHook();
    res = true;
  } else {
    st_log_error(_CONFIG_MANAGER_TAG, "Configuration drop failed");
  }
  return res;
}

bool ConfigManagerClass::saveConfig() {
  bool res = false;
  if (_config.size() == 0) {
    return res;
  }

  // todo probably can be optimized
  String data;

  for (auto it = _config.begin(); it != _config.end(); ++it) {
    ConfigEntry * entry = *it;

    if (entry->value() != nullptr && strlen(entry->value()) > 0) {
      String value = entry->value();
      value.replace(";", "|;");

      char buff[strlen(entry->name()) + value.length() + 3];
      sprintf(
        buff,
        "%s;%s%s",
        entry->name(),
        value.c_str(),
        it == std::prev(_config.end()) ? "" : ";"
      );

      data += String(buff);
    }
  }

  if (SettingsRepository.setConfig(data) >= 0) {
    res = true;
    st_log_debug(_CONFIG_MANAGER_TAG, "Configuration updated");
    callConfigUpdateHook();
  } else {
    st_log_error(_CONFIG_MANAGER_TAG, "Configuration update failed");
  }
  return res;
}

void ConfigManagerClass::loadConfigValues() {
  String data = SettingsRepository.getConfig();

  if (data.isEmpty()) {
    return;
  }

  String buff, key;
  char tmp;
  bool escaped = false, buildKey = true;
  for (uint8_t i = 0; i < data.length(); i++) {
    tmp = data.charAt(i);

    if (!escaped && tmp == '|') {
      escaped = true;
    } else if (!escaped && tmp == ';') {
      if (buildKey) {
        key = buff;
        buff.clear();
        buildKey = false;
      } else {
        setConfigValueWithoutSave(key.c_str(), buff.c_str());
        key.clear();
        buff.clear();
        buildKey = true;
      }
    } else {
      if (escaped && tmp != ';') {
        buff += '|';
      }
      buff += tmp;
      escaped = false;
    }
  }
  setConfigValueWithoutSave(key.c_str(), buff.c_str());
}

void ConfigManagerClass::onConfigUpdate(ConfigUpdatedHook hook) {
  _configUpdatedHook = hook;
}

void ConfigManagerClass::callConfigUpdateHook() {
  st_log_debug(_CONFIG_MANAGER_TAG, "Config updated, calling hooks");
  #if ENABLE_LOGGER
    LOGGER.updateAddress(ConfigManager.getConfigValue(LOGGER_ADDRESS_CONFIG));
  #endif
  _configUpdatedHook();
}