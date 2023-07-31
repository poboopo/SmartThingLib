#include "smartthing/watcher/CallbacksManager.h"
#include "smartthing/watcher/SensorWatcher.h"
#include "smartthing/watcher/DeviceStateWatcher.h"

#define CALLBACKS_MANAGER_TAG "callbacks_manager"


namespace Callback {
    using namespace Configurable::Sensor;
    using namespace Configurable::DeviceState;

    bool CallbacksManager::addSensorCallback(const Sensor * sensor, WatcherCallback<int16_t> * callback) {
        if (sensor == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Sensor object is missing, skipping...");
            return false;
        }
        if (callback == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback is missing, skipping...");
        }
        Watcher<Sensor, int16_t> * watcher = getWatcher<Sensor, int16_t>(&_sensorsWatchers, sensor);
        if (watcher == nullptr) {
            watcher = new SensorWatcher(sensor, callback);
            if (_sensorsWatchers.append(watcher) < 0) {
                LOGGER.error(CALLBACKS_MANAGER_TAG, "Failed to register new watcher for sensor [%s].", sensor->name);
                if (watcher != nullptr) {
                    delete(watcher);
                }
                return false;
            }
            LOGGER.info(CALLBACKS_MANAGER_TAG, "Registered new watcher for sensor [%s].", sensor->name);
        } else {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Watcher for sensor %s already exists!", sensor->name);

            watcher->addCallback(callback);
            LOGGER.info(CALLBACKS_MANAGER_TAG, "Added new callback for sensor %s", sensor->name);
        }
        return true;
    }

    bool CallbacksManager::addDeviceStateCallback(const DeviceState * state, WatcherCallback<char *> * callback) {
        if (state == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Device state object is missing, skipping...");
            return false;
        }
        if (callback == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback is missing, skipping...");
        }
        Watcher<DeviceState, char *> * watcher = getWatcher<DeviceState, char *>(&_statesWatchers, state);
        if (watcher == nullptr) {
            watcher = new DeviceStateWatcher(state, callback);
            if (_statesWatchers.append(watcher) < 0) {
                LOGGER.error(CALLBACKS_MANAGER_TAG, "Failed to register new watcher for state [%s].", state->name);
                if (watcher != nullptr) {
                    delete(watcher);
                }
                return false;
            }
            LOGGER.info(CALLBACKS_MANAGER_TAG, "Registered new watcher for state [%s].", state->name);
        } else {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Watcher for device state %s already exists!", state->name);

            watcher->addCallback(callback);
            LOGGER.info(CALLBACKS_MANAGER_TAG, "Added new callback for device sate %s", state->name);
        }
        return true;
    }

    template<typename O, typename T>
    Watcher<O, T> * CallbacksManager::getWatcher(List<Watcher<O, T>> * list, const O * observable) {
        return list->findValue([observable](Watcher<O, T> * current) {
            return current->getObservable() == observable;
        });
    }

    DynamicJsonDocument CallbacksManager::getWatchersInfo() {
        DynamicJsonDocument doc(1024);
        if (_sensorsWatchers.size() > 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Collecting info from sensors watchers");
            JsonArray array = doc.createNestedArray(SENSOR_WATCHER_TYPE);
            collectInfo<Sensor, int16_t>(&_sensorsWatchers, &array);
        }
        if (_statesWatchers.size() > 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Collecting info from device state watchers");
            JsonArray array = doc.createNestedArray(STATE_WATCHER_TYPE);
            collectInfo<DeviceState, char *>(&_statesWatchers, &array);
        }
        return doc;
    }

    DynamicJsonDocument CallbacksManager::getCallbacksJson(const char * type, const char * name) {
        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            Watcher<Sensor, int16_t> * watcher = _sensorsWatchers.findValue([&](Watcher<Sensor, int16_t> * current) {
                return strcmp(current->getObservable()->name, name) == 0;
            });
            if (watcher != nullptr) {
                return watcher->getCallbacksJson();
            }
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            Watcher<DeviceState, char *> * watcher = _statesWatchers.findValue([&](Watcher<DeviceState, char *> * current) {
                return strcmp(current->getObservable()->name, name) == 0;
            });
            if (watcher != nullptr) {
                return watcher->getCallbacksJson();
            }
        }
        LOGGER.error(CALLBACKS_MANAGER_TAG, "Watcher with type %s and index %d not found!", type, index);
        DynamicJsonDocument doc(16);
        return doc;
    }


    template<typename O, typename T>
    void CallbacksManager::collectInfo(List<Watcher<O, T>> * list, JsonArray * array) {
        list->forEach([&](Watcher<O, T> * current) {
            array->add(current->getObservableInfo());
        });
    }

    void CallbacksManager::check() {
        if (_sensorsWatchers.size() > 0) {
            checkWatchers<Sensor, int16_t>(&_sensorsWatchers);
        }
        if (_statesWatchers.size() > 0) {
            checkWatchers<DeviceState, char *>(&_statesWatchers);
        }
    }

    template<typename O, typename T>
    void CallbacksManager::checkWatchers(List<Watcher<O, T>> * list) {
        list->forEach([](Watcher<O, T> * current) {
            current->check();
        });
    }
}