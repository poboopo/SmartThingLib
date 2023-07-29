#include "smartthing/watcher/WatchersManager.h"
#include "smartthing/watcher/SensorWatcher.h"
#include "smartthing/watcher/DeviceStateWatcher.h"

#define WATCHERS_MANAGER_TAG "watchers_manager"


namespace Watcher {
    using namespace Configurable::Sensor;
    using namespace Configurable::DeviceState;

    bool WatchersManager::addSensorCallback(const Sensor * sensor, Callback::WatcherCallback<int16_t> * callback) {
        if (sensor == nullptr) {
            LOGGER.error(WATCHERS_MANAGER_TAG, "Sensor object is missing, skipping...");
            return false;
        }
        if (callback == nullptr) {
            LOGGER.error(WATCHERS_MANAGER_TAG, "Callback is missing, skipping...");
        }
        Watcher<Sensor, int16_t> * watcher = getWatcher<Sensor, int16_t>(&_sensorsWatchers, sensor);
        if (watcher == nullptr) {
            watcher = new SensorWatcher(sensor, callback);
            if (_sensorsWatchers.append(watcher) < 0) {
                LOGGER.error(WATCHERS_MANAGER_TAG, "Failed to register new watcher for sensor [%s].", sensor->name);
                if (watcher != nullptr) {
                    delete(watcher);
                }
                return false;
            }
            LOGGER.info(WATCHERS_MANAGER_TAG, "Registered new watcher for sensor [%s].", sensor->name);
        } else {
            LOGGER.debug(WATCHERS_MANAGER_TAG, "Watcher for sensor %s already exists!", sensor->name);

            watcher->addCallback(callback);
            LOGGER.info(WATCHERS_MANAGER_TAG, "Added new callback for sensor %s", sensor->name);
        }
        return true;
    }

    bool WatchersManager::addDeviceStateCallback(const DeviceState * state, Callback::WatcherCallback<char *> * callback) {
        if (state == nullptr) {
            LOGGER.error(WATCHERS_MANAGER_TAG, "Device state object is missing, skipping...");
            return false;
        }
        if (callback == nullptr) {
            LOGGER.error(WATCHERS_MANAGER_TAG, "Callback is missing, skipping...");
        }
        Watcher<DeviceState, char *> * watcher = getWatcher<DeviceState, char *>(&_statesWatchers, state);
        if (watcher == nullptr) {
            watcher = new DeviceStateWatcher(state, callback);
            if (_statesWatchers.append(watcher) < 0) {
                LOGGER.error(WATCHERS_MANAGER_TAG, "Failed to register new watcher for state [%s].", state->name);
                if (watcher != nullptr) {
                    delete(watcher);
                }
                return false;
            }
            LOGGER.info(WATCHERS_MANAGER_TAG, "Registered new watcher for state [%s].", state->name);
        } else {
            LOGGER.debug(WATCHERS_MANAGER_TAG, "Watcher for device state %s already exists!", state->name);

            watcher->addCallback(callback);
            LOGGER.info(WATCHERS_MANAGER_TAG, "Added new callback for device sate %s", state->name);
        }
        return true;
    }

    template<typename O, typename T>
    Watcher<O, T> * WatchersManager::getWatcher(List<Watcher<O, T>> * list, const O * observable) {
        return list->findValue([observable](Watcher<O, T> * current) {
            return current->getObservable() == observable;
        });
    }

    DynamicJsonDocument WatchersManager::getWatchersInfo() {
        DynamicJsonDocument doc(1024);
        if (_sensorsWatchers.size() > 0) {
            LOGGER.debug(WATCHERS_MANAGER_TAG, "Collecting info from sensors watchers");
            collectInfo<Sensor, int16_t>(&_sensorsWatchers, &doc);
        }
        if (_statesWatchers.size() > 0) {
            LOGGER.debug(WATCHERS_MANAGER_TAG, "Collecting info from device state watchers");
            collectInfo<DeviceState, char *>(&_statesWatchers, &doc);
        }
        return doc;
    }

    template<typename O, typename T>
    void WatchersManager::collectInfo(List<Watcher<O, T>> * list, DynamicJsonDocument * doc) {
        list->forEach([&](Watcher<O, T> * current) {
            doc->add(current->getInfo());
        });
    }

    void WatchersManager::check() {
        if (_sensorsWatchers.size() > 0) {
            checkWatchers<Sensor, int16_t>(&_sensorsWatchers);
        }
        if (_statesWatchers.size() > 0) {
            checkWatchers<DeviceState, char *>(&_statesWatchers);
        }
    }

    template<typename O, typename T>
    void WatchersManager::checkWatchers(List<Watcher<O, T>> * list) {
        list->forEach([](Watcher<O, T> * current) {
            current->check();
        });
    }
}