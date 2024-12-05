#include "observable/ObservablesManager.h"
#include "logs/BetterLogger.h"

ObservablesManagerClass ObservablesManager;

const char * const _OBSERVABLES_MANAGER_TAG = "observables-manager";

#if ENABLE_SENSORS

template<>
const ObservableObject<SENSOR_DATA_TYPE> * ObservablesManagerClass::getObservableObject(const char * name) {
  return _sensorsList.findValue([&](ObservableObject<SENSOR_DATA_TYPE> * current) {
      return strcmp(current->name, name) == 0;
  });
}

bool ObservablesManagerClass::addSensor(const char* name, SensorType type, ObservableObject<SENSOR_DATA_TYPE>::ValueProviderFunction valueProvider) {
  if (getObservableObject<SENSOR_DATA_TYPE>(name) != nullptr) {
    st_log_warning(_OBSERVABLES_MANAGER_TAG,
                    "Sensor with name %s already exist! Skipping...", name);
    return false;
  }
  Sensor* sensor = new Sensor();
  sensor->name = name; // todo what if name pointer freed?
  sensor->valueProvider = valueProvider;
  sensor->sensorType = type;
  if (_sensorsList.append(sensor) > -1) {
    st_log_debug(_OBSERVABLES_MANAGER_TAG, "Added new sensor %s", name);
    return true;
  } else {
    if (sensor != nullptr) {
      delete sensor;
    }
    st_log_error(_OBSERVABLES_MANAGER_TAG, "Failed to add new sensor %s", name);
    return false;
  }
};

bool ObservablesManagerClass::addSensorCustom(const char* name, ObservableObject<SENSOR_DATA_TYPE>::ValueProviderFunction function) {
  return addSensor(name, TYPE_CUSTOM, function);
}

bool ObservablesManagerClass::addDigitalSensor(const char* name, uint8_t pin, uint8_t mode) {
  pinMode(pin, mode);
  return addSensor(name, TYPE_DIGITAL, [pin]() {
    if (pin > 0) {
      return digitalRead(pin);
    }
    return -1;
  });
}

bool ObservablesManagerClass::addAnalogSensor(const char* name, uint8_t pin) {
  return addSensor(name, TYPE_ANALOG, [pin]() {
    if (pin > 0) {
      return (int)analogRead(pin);
    }
    return -1;
  });
}

uint8_t ObservablesManagerClass::getSensorsCount() { return _sensorsList.size(); }

#endif

#if ENABLE_STATES

template<>
const ObservableObject<STATE_DATA_TYPE> * ObservablesManagerClass::getObservableObject(const char * name) {
  return _deviceStatesList.findValue([&](ObservableObject<STATE_DATA_TYPE> * current) {
      return strcmp(current->name, name) == 0;
  });
}

bool ObservablesManagerClass::addDeviceState(const char* name, ObservableObject<STATE_DATA_TYPE>::ValueProviderFunction valueProvider) {
  if (getObservableObject<STATE_DATA_TYPE>(name) != nullptr) {
      st_log_warning(_OBSERVABLES_MANAGER_TAG, "State with name %s already exist! Skipping...", name);
      return false;
    }
    DeviceState *state = new DeviceState();
    state->name = name; // todo what if name pointer freed? 
    state->valueProvider = valueProvider;
    if (_deviceStatesList.append(state) > -1) {
      st_log_debug(_OBSERVABLES_MANAGER_TAG, "Added new device state %s", name);
      return true;
    } else {
      if (state != nullptr) {
        delete state;
      }
      st_log_error(_OBSERVABLES_MANAGER_TAG, "Dailed to add new device state %s", name);
      return false;
    }
}

uint8_t ObservablesManagerClass::getDeviceStatesCount() {
  return _deviceStatesList.size();
}

#endif

template<typename T>
const ObservableObject<T> * ObservablesManagerClass::getObservableObject(const char * name) {
  return nullptr;
}

JsonDocument ObservablesManagerClass::getObservablesInfo(bool full) {
  JsonDocument doc;
  
  if (full) {
    doc.to<JsonArray>();
    JsonArray array = doc.as<JsonArray>();

    #if ENABLE_SENSORS
      collectObservablesInfoFull<SENSOR_DATA_TYPE>(&_sensorsList, array, OBS_SENSOR);
    #endif

    #if ENABLE_STATES
      collectObservablesInfoFull<STATE_DATA_TYPE>(&_deviceStatesList, array, OBS_STATE);
    #endif
  } else {
    doc.to<JsonObject>();
    JsonObject object = doc.as<JsonObject>();

    #if ENABLE_SENSORS
      _sensorsList.forEach([&](ObservableObject<SENSOR_DATA_TYPE> * obj) {
        object[obj->name] = obj->valueProvider();
      });
    #endif

    #if ENABLE_STATES
    _deviceStatesList.forEach([&](ObservableObject<STATE_DATA_TYPE> * obj) {
      object[obj->name] = obj->valueProvider();
    });
    #endif
  }

  return doc;
}

template<typename T>
void ObservablesManagerClass::collectObservablesInfoFull(
  List<ObservableObject<T>> * values,
  JsonArray &array,
  ObservableType type
) {
  const char * typeStr = observableTypeToStr(type);

  values->forEach([&](ObservableObject<T> * obj) {
    JsonDocument doc;
    doc["name"] = obj->name;
    doc["value"] = obj->valueProvider();
    doc["type"] = typeStr;

    array.add(doc);
  });
}