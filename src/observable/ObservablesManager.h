#ifndef OBSERVABLES_MANAGER_H
#define OBSERVABLES_MANAGER_H

#include "Features.h"
#include "observable/SensorsList.h"
#include "observable/DeviceStatesList.h"

#if ENABLE_SENSORS || ENABLE_STATES

class ObservablesManagerClass {
  public:
    #if ENABLE_SENSORS
    bool addSensor(
        const char* name,
        ObservableObject<int16_t>::ValueProviderFunction valueProvider
      );
    bool addDigitalSensor(const char* name, int pin);
    bool addAnalogSensor(const char* name, int pin);
    const Sensor* getSensor(const char* name);
    JsonDocument getSensorsValues();
    JsonDocument getSensorsTypes();
    int16_t getSensorsCount();
    #endif

    #if ENABLE_STATES
    bool addDeviceState(const char* name, ObservableObject<const char*>::ValueProviderFunction valueProvider);

    const DeviceState* getDeviceState(
        const char* name);
    JsonDocument getDeviceStatesInfo();

    int16_t getDeviceStatesCount();
    #endif
  private:
    #if ENABLE_SENSORS
    SensorsList _sensorsList;
    #endif

    #if ENABLE_STATES
    DeviceStatesList _deviceStatesList;
    #endif
};

extern ObservablesManagerClass ObservablesManager;

#endif

#endif