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

    bool CallbacksManager::deleteCallback(const char * type, const char * name, int16_t index) {
        if (type == nullptr || strlen(type) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Type of observable is missing!");
            return false;
        }

        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            return deleteWatcherCallbackFromList<Sensor, int16_t>(&_sensorsWatchers, name, index);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return deleteWatcherCallbackFromList<DeviceState, char *>(&_statesWatchers, name, index);
        }

        LOGGER.error(CALLBACKS_MANAGER_TAG, "Type %s not supported", type);
        return false;
    }

    bool CallbacksManager::updateCallback(const char * type, const char * name, int16_t index, const char * json) {
        if (type == nullptr || strlen(type) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Type of observable is missing!");
            return false;
        }

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);

        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            WatcherCallback<int16_t> * callback = getCallbackFromWatcherList(&_sensorsWatchers, name, index);
            if (callback == nullptr) {
                return false;
            }
            if (callback->isReadonly()) {
                LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback %d for observable %s is readonly!", index, name);
                return false;
            }

            if (doc.containsKey("trigger")) {
                int16_t newValue = doc["trigger"];
                int16_t * oldValue = callback->triggerValuePointer();
                memcpy(oldValue, &newValue, sizeof(*oldValue));
            }
            callback->updateCustom(doc);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            WatcherCallback<char *> * callback = getCallbackFromWatcherList(&_statesWatchers, name, index);
            if (callback == nullptr) {
                return false;
            }
            if (callback->isReadonly()) {
                LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback %d for observable %s is readonly!", index, name);
                return false;
            }

            if (doc.containsKey("trigger")) {
                const char * newValue = doc["trigger"];
                int size = strlen(newValue);
                char * oldValue = (*callback->triggerValuePointer());
                delete(oldValue);
                oldValue = new char[size + 1];
                strncpy(oldValue, newValue, size + 1);
                oldValue[size] = '\0';
            }

            callback->updateCustom(doc);
        } else {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Observable type [%s] not supported!", type);
            return false;
        }

        LOGGER.info(CALLBACKS_MANAGER_TAG, "Callback №%d for [%s]::%s was updated!", index, type, name);
        return true;
    }


    template<typename O, typename T>
    WatcherCallback<T> * CallbacksManager::getCallbackFromWatcherList(List<Watcher<O, T>> * list, const char * name, int16_t callbackIndex) {
        Watcher<O, T> * watcher = getWatcherByObservableName(list, name);
        if (watcher == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Can't find watcher for observable %s", name);
            return nullptr;
        }
        WatcherCallback<T> * callback = watcher->getCallback(callbackIndex);
        if (callback == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Can't find callback %d for observable %s", index, name);
            return nullptr;
        }
        return callback;
    }

    template<typename O, typename T>
    bool CallbacksManager::deleteWatcherCallbackFromList(List<Watcher<O, T>> * list, const char * name, int16_t index) {
        if (name == nullptr || strlen(name) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Name of observable is missing!");
            return false;
        }
        Watcher<O, T> * watcher = getWatcherByObservableName(list, name);
        if (watcher == nullptr) {
            return false;
        }
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Trying to delete %s's callback %d", name, index);
        if (!watcher->removeCallback(index)) {
            return false;
        }
        LOGGER.warning(CALLBACKS_MANAGER_TAG, "Callback %d of %s was deleted", index, name);
        if (watcher->haveCallbacks()) {
            return true;
        }
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "No callbacks left for %s, removing watcher!", name);
        if (list->remove(watcher)) {
            delete(watcher);
            LOGGER.warning(CALLBACKS_MANAGER_TAG, "Watcher for %s removed!", name);
        }
        return true;
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
            return getCallbacksJsonFromList<Sensor, int16_t>(&_sensorsWatchers, name);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return getCallbacksJsonFromList<DeviceState, char *>(&_statesWatchers, name);

        }
        LOGGER.error(CALLBACKS_MANAGER_TAG, "Type %s not supported", type);
        DynamicJsonDocument doc(4);
        return doc;
    }

    template<typename O, typename T>
    DynamicJsonDocument CallbacksManager::getCallbacksJsonFromList(List<Watcher<O, T>> * list, const char * name) {
        if (name == nullptr || strlen(name) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Name of observable is missing!");
        } else {
            Watcher<O, T> * watcher = getWatcherByObservableName(list, name);
            if (watcher != nullptr) {
                return watcher->getCallbacksJson();
            } else {
                LOGGER.error(CALLBACKS_MANAGER_TAG, "Can't find watcher for observable %s", name);
            }
        }
        DynamicJsonDocument doc(4);
        return doc;
    }

    template<typename O, typename T>
    Watcher<O, T> * CallbacksManager::getWatcher(List<Watcher<O, T>> * list, const O * observable) {
        return list->findValue([observable](Watcher<O, T> * current) {
            return current->getObservable() == observable;
        });
    }

    template<typename O, typename T>
    Watcher<O, T> * CallbacksManager::getWatcherByObservableName(List<Watcher<O, T>> * list, const char * name) {
        return list->findValue([name](Watcher<O, T> * current) {
            return strcmp(current->getObservable()->name, name) == 0;
        });
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