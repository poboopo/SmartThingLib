#ifndef CONFIG_ENTRIES_LIST 
#define CONFIG_ENTRIES_LIST 

#include <ArduinoJson.h>

namespace Configurable {
    namespace Config {
        struct ConfigEntry {
            const char * name;
            const char * caption;
            const char * type;

            ConfigEntry * next;
            ConfigEntry * previous;
        };

        class ConfigEntriesList {
            public:
                ConfigEntriesList(): _head(nullptr){};
                ~ConfigEntriesList();

                bool add(const char * name, const char * caption, const char * type);
                DynamicJsonDocument getDict();

                int size() {
                    return _count;
                }
            private:
                ConfigEntry * _head;
                int _count = 0;
                void append(ConfigEntry * sensor);
                const ConfigEntry * findConfigEntry(const char * name) const;

        };
    }
}

#endif