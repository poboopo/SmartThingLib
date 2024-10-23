#ifndef CONFIG_ENTRIES_LIST
#define CONFIG_ENTRIES_LIST

#include <ArduinoJson.h>

#include "utils/List.h"
#include "logs/BetterLogger.h"

static const char * CONFIG_ENTRIES_LIST_TAG = "config_entries_list";

namespace Config {
struct ConfigEntry {
  ConfigEntry(const char* n, const char* c, const char* t)
      : name(n), caption(c), type(t){};
  const char* name;
  const char* caption;
  const char* type;
};

class ConfigEntriesList : public List<ConfigEntry> {
 public:
  bool add(const char* name, const char* caption, const char* type) {
    if (findConfigEntry(name) != nullptr) {
      SMT_LOG_WARNING(CONFIG_ENTRIES_LIST_TAG, "Config entry %s already exists!",
                     name);
      return false;
    }
    ConfigEntry* entry = new ConfigEntry(name, caption, type);
    if (append(entry) > -1) {
      SMT_LOG_DEBUG(CONFIG_ENTRIES_LIST_TAG, "Added new config entry - %s:%s",
                   name, caption);
      return true;
    } else {
      if (entry != nullptr) {
        delete entry;
      }
      SMT_LOG_ERROR(CONFIG_ENTRIES_LIST_TAG,
                   "Failed to add new config entry - %s:%s", name, caption);
      return false;
    }
  };
  JsonDocument toJson() {
    JsonDocument doc;
    forEach([&](ConfigEntry* current) {
      JsonObject obj = doc[current->name].to<JsonObject>();
      obj["caption"] = current->caption;
      obj["type"] = current->type;
    });
    return doc;
  }
  bool haveConfigEntry(const char * name) {
    return findConfigEntry(name) != nullptr;
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
}  // namespace Config

#endif