#ifndef CONFIG_OBJECTS_H
#define CONFIG_OBJECTS_H

#include <functional>

namespace Configurable {
    namespace Sensor {
        enum SensorType {
            TYPE_CUSTOM,
            TYPE_DIGITAL,
            TYPE_ANALOG
        };

        typedef std::function<uint16_t(void)> ValueGeneratorFunction;

        // add units?
        struct Sensor {
            int pin;
            const char * name;
            SensorType type;
            ValueGeneratorFunction valueGenerator = [](){return 0;};

            Sensor * next;
            Sensor * previous;
        };
    }
    namespace DeviceState {
        typedef std::function<const char *(void)> ValueGeneratorFunction;

        struct DeviceState {
            const char * name;
            ValueGeneratorFunction valueGenerator = [](){return "";};

            DeviceState * next;
            DeviceState * previous;
        };
    }
}

#endif