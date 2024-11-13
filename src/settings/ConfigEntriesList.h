#ifndef CONFIG_ENTRIES_LIST
#define CONFIG_ENTRIES_LIST

#include <ArduinoJson.h>

#include "utils/List.h"
#include "logs/BetterLogger.h"

static const char * _CONFIG_ENTRIES_LIST_TAG = "config_entries_list";

enum ConfigEntryType {
  CONFIG_STRING,
  CONFIG_INTEGER,
  CONFIG_BOOLEAN
};

struct ConfigEntry {
  ConfigEntry(const char* n, const char* c, ConfigEntryType t)
      : name(n), caption(c), type(t){};
  const char* name;
  const char* caption;
  ConfigEntryType type;
};

class ConfigEntriesList : public List<ConfigEntry> {
 public:
  bool add(const char* name, const char* caption, ConfigEntryType type) {
    if (findConfigEntry(name) != nullptr) {
      st_log_warning(_CONFIG_ENTRIES_LIST_TAG, "Config entry %s already exists!", name);
      return false;
    }
    ConfigEntry* entry = new ConfigEntry(name, caption, type);
    if (append(entry) > -1) {
      st_log_debug(_CONFIG_ENTRIES_LIST_TAG, "Added new config entry - %s:%s",
                   name, caption);
      return true;
    } else {
      if (entry != nullptr) {
        delete entry;
      }
      st_log_error(_CONFIG_ENTRIES_LIST_TAG,
                   "Failed to add new config entry - %s:%s", name, caption);
      return false;
    }
  };
  JsonDocument toJson() {
    JsonDocument doc;
    forEach([&](ConfigEntry* current) {
      JsonObject obj = doc[current->name].to<JsonObject>();
      obj["caption"] = current->caption;
      switch (current->type) {
        case CONFIG_STRING:
          obj["type"] = "string";
          break;
        case CONFIG_INTEGER:
          obj["type"] = "number";
          break;
        case CONFIG_BOOLEAN:
          obj["type"] = "boolean";
          break;
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
        [&](ConfigEntry* current) { return strcmp(current->name, name) == 0; });
  };
};

#endif