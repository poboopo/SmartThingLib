#ifndef CONFIG_ENTRIES_LIST
#define CONFIG_ENTRIES_LIST

#include <ArduinoJson.h>

#include "utils/List.h"
#include "logs/BetterLogger.h"

const char * const _CONFIG_ENTRIES_LIST_TAG = "config_entries_list";

enum ConfigEntryType {
  CONFIG_STRING,
  CONFIG_INTEGER,
  CONFIG_BOOLEAN
};

class ConfigEntry {
  public:
    ConfigEntry(const char* name, ConfigEntryType t)
        : _type(t) {
      _name = (char*) malloc(strlen(name) + 1);
      strcpy(_name, name);
    };
    ~ConfigEntry() {
      free(_name);
    }

    const char * name() const {
      return _name;
    }

    ConfigEntryType type() const {
      return _type;
    }

  private:
    char* _name;
    ConfigEntryType _type;
};

class ConfigEntriesList : public List<ConfigEntry> {
 public:
  bool add(const char* name, ConfigEntryType type) {
    if (findConfigEntry(name) != nullptr) {
      st_log_warning(_CONFIG_ENTRIES_LIST_TAG, "Config entry %s already exists!", name);
      return false;
    }
    ConfigEntry* entry = new ConfigEntry(name, type);
    if (append(entry) > -1) {
      st_log_debug(_CONFIG_ENTRIES_LIST_TAG, "Added new config entry - %s", name);
      return true;
    } else {
      if (entry != nullptr) {
        delete entry;
      }
      st_log_error(_CONFIG_ENTRIES_LIST_TAG, "Failed to add new config entry - %s", name);
      return false;
    }
  };
  JsonDocument toJson() {
    JsonDocument doc;
    forEach([&](ConfigEntry* current) {
      JsonObject obj = doc[current->name()].to<JsonObject>();
      switch (current->type()) {
        case CONFIG_INTEGER:
          obj["type"] = "number";
          break;
        case CONFIG_BOOLEAN:
          obj["type"] = "boolean";
          break;
        default:
          obj["type"] = "string";
      }
    });
    return doc;
  }
 private:
  const ConfigEntry* findConfigEntry(const char* name) {
    if (strlen(name) == 0) {
      return nullptr;
    }
    return findValue(
        [&](ConfigEntry* current) { return strcmp(current->name(), name) == 0; });
  };
};

#endif