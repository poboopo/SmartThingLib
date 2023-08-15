#include "smartthing/watcher/CallbacksManager.h"
#include "smartthing/watcher/SensorWatcher.h"
#include "smartthing/watcher/DeviceStateWatcher.h"
#include "smartthing/SmartThing.h"
#include "smartthing/watcher/callback/CallbackBuilder.h"

#define CALLBACKS_MANAGER_TAG "callbacks_manager"

namespace Callback {
    using namespace Configurable::Sensor;
    using namespace Configurable::DeviceState;

    bool CallbacksManager::createCallbacksFromJson(const char * json) {
        if (json == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Json is null!");
            return false;
        }
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Creating callback from json: %s", json);
        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);
        if (doc.size() == 0) {
            return false;
        }
        if (doc.memoryUsage() == 0) {
            return false;
        }

        if (!doc.containsKey("observableType")) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "type value is missing!");
            return false;
        }
        if (!doc.containsKey("observable")) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "observable value is missing!");
            return false;
        }

        const char * observableType = doc["observableType"];
        const char * obs = doc["observable"];
        if (observableType == nullptr || obs == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Parameters observableType or observable are missing!");
            return false;
        }

        CallbackBuilder builder;
        builder.url(doc["url"])->method(doc["method"])->payload(doc["payload"]);
        String trigger = doc["trigger"];
        if (strcmp(observableType, STATE_TYPE) == 0) {
            return addDeviceStateCallback(SmartThing.getDeviceState(obs), builder.build<String>(trigger));
        } else if (strcmp(observableType, SENSOR_TYPE) == 0) {
            int triggerValue;
            if (trigger.length() == 0) {
                triggerValue = -1;
            } else {
                triggerValue = trigger.toInt();
            }
            return addSensorCallback(SmartThing.getSensor(obs), builder.build<int16_t>(triggerValue));
        }

        LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback for observable[%s] of type %s not supported. Supported types: state, sensor.", obs, observableType);
        return false;
    }

    DynamicJsonDocument CallbacksManager::callbacksToJson() {
        int size = _callbacksCount * CALLBACK_INFO_DOC_SIZE + 
            (_statesWatchers.size() + _sensorsWatchers.size()) * WATCHER_INFO_DOC_SIZE;

        DynamicJsonDocument doc(size);
        _sensorsWatchers.forEach([&](Watcher<Sensor, int16_t> * watcher){
            doc.add(watcher->toJson());
        });
        _statesWatchers.forEach([&](Watcher<DeviceState, String> * watcher){
            doc.add(watcher->toJson());
        });
        return doc;
    }

    bool CallbacksManager::addSensorCallback(const char * name, LambdaCallback<int16_t>::CustomCallback callback, int16_t triggerValue) {
        const Sensor * sensor = SmartThing.getSensor(name);
        if (sensor == nullptr) {
            LOGGER.error(SMART_THING_TAG, "Can't find sensor with name %s. Not registered yet?", name);
            return false;
        }
        LambdaCallback<int16_t> * watcherCallback = new LambdaCallback<int16_t>(callback, triggerValue);
        return addSensorCallback(sensor, watcherCallback);
    }

    bool CallbacksManager::addSensorCallback(const char * name, const char * url, int16_t triggerValue, bool readonly) {
        const Sensor * sensor = SmartThing.getSensor(name);
        if (sensor == nullptr) {
            LOGGER.error(SMART_THING_TAG, "Can't find sensor with name %s. Not registered yet?", name);
            return false;
        }
        HttpCallback<int16_t> * watcherCallback = new HttpCallback<int16_t>(url, triggerValue, readonly);
        return addSensorCallback(sensor, watcherCallback);
    }

    bool CallbacksManager::addDeviceStateCallback(const char * name, LambdaCallback<String>::CustomCallback callback, const char * triggerValue) {
        const DeviceState * state = SmartThing.getDeviceState(name);
        if (state == nullptr) {
            LOGGER.error(SMART_THING_TAG, "Can't find device state with name %s. Not registered yet?", name);
            return false;
        }
        LambdaCallback<String> * watcherCallback = new LambdaCallback<String>(callback, triggerValue);
        return addDeviceStateCallback(state, watcherCallback);
    }

    bool CallbacksManager::addDeviceStateCallback(const char * name, const char * url, const char * triggerValue, bool readonly) {
        const DeviceState * state = SmartThing.getDeviceState(name);
        if (state == nullptr) {
            LOGGER.error(SMART_THING_TAG, "Can't find device state with name %s. Not registered yet?", name);
            return false;
        }
        HttpCallback<String> * watcherCallback = new HttpCallback<String>(url, triggerValue, readonly);
        return addDeviceStateCallback(state, watcherCallback);
    }

    bool CallbacksManager::addSensorCallback(const Sensor * sensor, WatcherCallback<int16_t> * callback) {
        if (sensor == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Sensor object is missing, skipping...");
            return false;
        }
        if (callback == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback is missing, skipping...");
            return false;
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
            _callbacksCount++;
            LOGGER.info(CALLBACKS_MANAGER_TAG, "Added new callback for sensor %s", sensor->name);
        }
        return true;
    }

    bool CallbacksManager::addDeviceStateCallback(const DeviceState * state, WatcherCallback<String> * callback) {
        if (state == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Device state object is missing, skipping...");
            return false;
        }
        if (callback == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback is missing, skipping...");
            return false;
        }
        Watcher<DeviceState, String> * watcher = getWatcher<DeviceState, String>(&_statesWatchers, state);
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
            _callbacksCount++;
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
            return deleteWatcherCallbackFromList<DeviceState, String>(&_statesWatchers, name, index);
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
                callback->setTriggerValue(doc["trigger"]);
            }
            callback->updateCustom(doc);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            WatcherCallback<String> * callback = getCallbackFromWatcherList(&_statesWatchers, name, index);
            if (callback == nullptr) {
                return false;
            }
            if (callback->isReadonly()) {
                LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback %d for observable %s is readonly!", index, name);
                return false;
            }

            if (doc.containsKey("trigger")) {
                callback->setTriggerValue(doc["trigger"]);
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
        _callbacksCount--;
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
            collectInfo<DeviceState, String>(&_statesWatchers, &array);
        }
        return doc;
    }

    DynamicJsonDocument CallbacksManager::getCallbacksJson(const char * type, const char * name) {
        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            return getCallbacksJsonFromList<Sensor, int16_t>(&_sensorsWatchers, name);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return getCallbacksJsonFromList<DeviceState, String>(&_statesWatchers, name);

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
            checkWatchers<DeviceState, String>(&_statesWatchers);
        }
    }

    template<typename O, typename T>
    void CallbacksManager::checkWatchers(List<Watcher<O, T>> * list) {
        list->forEach([](Watcher<O, T> * current) {
            current->check();
        });
    }
}