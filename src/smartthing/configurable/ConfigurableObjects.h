#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include <functional>

namespace Configurable {
    template<typename T>
    class ConfigurableObject {
        public:
            typedef std::function<T(void)> ValueGeneratorFunction;

            const char * name;
            ValueGeneratorFunction valueGenerator;
    };

    namespace Sensor {
        enum SensorType {
            TYPE_CUSTOM,
            TYPE_DIGITAL,
            TYPE_ANALOG
        };
        inline const char * sensorTypeName(SensorType type) {
            switch (type) {
                case TYPE_CUSTOM:
                    return "custom";
                case TYPE_DIGITAL:
                    return "digital";
                case TYPE_ANALOG:
                    return "analog";
            }
            return "type_not_found_how";
        };

        class Sensor: public ConfigurableObject<int16_t> {
            public:
                int pin;
                SensorType type;
        };
    }
    namespace DeviceState {
        class DeviceState: public ConfigurableObject<const char *> {};
    }
}

#endif