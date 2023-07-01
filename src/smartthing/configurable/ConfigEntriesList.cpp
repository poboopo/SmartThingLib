#include "smartthing/configurable/ConfigEntriesList.h"
#include "smartthing/logs/BetterLogger.h"

#define CONFIG_ENTRIES_LIST_TAG "config_entries_list"

using namespace Configurable::Config;

ConfigEntriesList::~ConfigEntriesList() {
    ConfigEntry * current = _head;
    while (current->next != nullptr) {
        current = current->next;
        delete(current->previous);
    }
    delete(current);
}

DynamicJsonDocument ConfigEntriesList::getDict() {
    DynamicJsonDocument doc(_count * 64);
    ConfigEntry * current = _head;
    while (current != nullptr) {
        JsonObject obj = doc.createNestedObject(current->name);
        obj["caption"] = current->caption;
        obj["type"] = current->type;

        current = current->next;
    }
    return doc;
}

bool ConfigEntriesList::add(const char * name, const char * caption, const char * type) {
    if (findConfigEntry(name) != nullptr) {
        LOGGER.warning(CONFIG_ENTRIES_LIST_TAG, "Config entry %s already exists!", name);
        return false;
    }
    ConfigEntry * action = new ConfigEntry;
    action->name = name;
    action->caption = caption;
    action->type = type;
    append(action);
    LOGGER.debug(CONFIG_ENTRIES_LIST_TAG, "Added new config entry - %s:%s", name, caption);
    return true;
}

void ConfigEntriesList::append(ConfigEntry * action) {
    action->next = _head;
    if (_head != nullptr) {
        _head->previous = action;
    }
    _head = action;
    _count++;
}

const ConfigEntry * ConfigEntriesList::findConfigEntry(const char * name) const {
    ConfigEntry * current = _head;
    while (current != nullptr) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}