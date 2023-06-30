#ifndef STATE_H
#define STATE_H

#include <functional>

class DeviceState {
    public:
    // todo add boolean and integer support
        typedef std::function<const char*(void)> ValueGeneratorFunction;
        DeviceState(const char * name, ValueGeneratorFunction valueGenerator): _name(name), _valueGenerator(valueGenerator){};

        const char * getName() const {
            return _name;
        }

        const char * getValue() const {
            return _valueGenerator();
        }

        DeviceState * next;
        DeviceState * previous;
    private:
        const char * _name;
        ValueGeneratorFunction _valueGenerator = [](){return "";};
};

#endif