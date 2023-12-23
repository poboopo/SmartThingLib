#include "callbacks/CallbacksManager.h"
#include "callbacks/watchers/SensorWatcher.h"
#include "callbacks/watchers/DeviceStateWatcher.h"
#include "SmartThing.h"
#include "callbacks/builders/CallbackBuilder.h"
#include "settings/SettingsManager.h"

// #include <unordered_map>

#define CALLBACKS_MANAGER_TAG "callbacks_manager"

Callback::CallbacksManagerClass CallbacksManager;

// std::unordered_map<int, int> m;

namespace Callback {
    using namespace Configurable::Sensor;
    using namespace Configurable::DeviceState;
    using Configurable::ConfigurableObject;

    void CallbacksManagerClass::loadFromSettings() {
        JsonArray callbacksInfo = STSettings.getCallbacks();
        if (callbacksInfo.size() == 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "There is not callbacks in settings");
            return;
        }

        for (int i = 0; i < callbacksInfo.size(); i++) {
            JsonObject observable = callbacksInfo[i]["observable"];
            JsonArray callbacks = callbacksInfo[i]["callbacks"];
            for (int j = 0; j < callbacks.size(); j++) {
                createCallbackFromJson(observable, callbacks[j]);
            }
        }
    }

    int CallbacksManagerClass::createCallbackFromJson(const char * json) {
        if (json == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Json is null!");
            return -1;
        }
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Creating callback from json: %s", json);
        
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, json);

        return createCallbackFromJson(doc["observable"], doc["callback"]);        
    }

    int CallbacksManagerClass::createCallbackFromJson(JsonObject observableInfo, JsonObject callback) {
        if (observableInfo.size() == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "ObservableInfo object is empty!");
            return -1;
        }
        if (callback.size() == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback object is empty!");
            return -1;
        }

        if (!observableInfo.containsKey("type")) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "observable type value is missing!");
            return -1;
        }
        if (!observableInfo.containsKey("name")) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "observable name value is missing!");
            return -1;
        }

        const char * observableType = observableInfo["type"];
        const char * name = observableInfo["name"];
        if (observableType == nullptr || name == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Parameters observableType or observable are missing!");
            return -1;
        }

        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Creating new callback for [%s]:%s", observableType, name);

        CallbackBuilder builder;
        builder.id(callback["id"])
            ->type(callback["type"])
            ->url(callback["url"])
            ->method(callback["method"])
            ->payload(callback["payload"])
            ->action(callback["action"])
            ->compareType(callback["compareType"]);

        String trigger = callback["trigger"];
        builder.triggerDisabled(trigger.isEmpty());

        if (strcmp(observableType, STATE_TYPE) == 0) {
            return addCallback(SmartThing.getDeviceState(name), builder.build<String>(trigger));
        } else if (strcmp(observableType, SENSOR_TYPE) == 0) {
            int triggerValue;
            if (trigger.length() == 0) {
                triggerValue = -1;
            } else {
                triggerValue = trigger.toInt();
            }
            return addCallback(SmartThing.getSensor(name), builder.build<int16_t>(triggerValue));
        }

        LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback for [%s] of type [%s] not supported. Supported types: state, sensor.", name, observableType);
        return -1;
    }

    DynamicJsonDocument CallbacksManagerClass::allCallbacksToJson(bool ignoreReadOnly, bool shortJson) {
        if (_callbacksCount == 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "No callbacks, creating empty doc");
            DynamicJsonDocument doc(0);
            doc.to<JsonArray>();
            return doc;
        }

        int size = _callbacksCount * CALLBACK_INFO_DOC_SIZE + 
            (_statesWatchers.size() + _sensorsWatchers.size()) * WATCHER_INFO_DOC_SIZE;

        if (size == 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "DynamicJsonDocument size = 0, creating empty doc");
            DynamicJsonDocument doc(0);
            return doc;
        }

        LOGGER.debug(CALLBACKS_MANAGER_TAG, "DynamicJsonDoc size for callbacks = %d", size);
        DynamicJsonDocument doc(size);
        _sensorsWatchers.forEach([&](Watcher<int16_t> * watcher){
            if (watcher == nullptr) {
                return;
            }
            DynamicJsonDocument wjs = watcher->toJson(ignoreReadOnly, shortJson);
            if (wjs.size() > 0)
                doc.add(wjs);
        });
        _statesWatchers.forEach([&](Watcher<String> * watcher){
            if (watcher == nullptr) {
                return;
            }
            DynamicJsonDocument wjs = watcher->toJson(ignoreReadOnly, shortJson);
            if (wjs.size() > 0)
                doc.add(wjs);
        });
        return doc;
    }

    int CallbacksManagerClass::addSensorCallback(const char * name, LambdaCallback<int16_t>::CustomCallback callback, int16_t triggerValue) {
        const Sensor * sensor = SmartThing.getSensor(name);
        if (sensor == nullptr) {
            LOGGER.error(SMART_THING_TAG, "Can't find sensor with name [%s]. Not registered yet?", name);
            return -1;
        }
        LambdaCallback<int16_t> * watcherCallback = new LambdaCallback<int16_t>(callback, triggerValue);
        return addCallback(sensor, watcherCallback);
    }

    int CallbacksManagerClass::addDeviceStateCallback(const char * name, LambdaCallback<String>::CustomCallback callback, const char * triggerValue) {
        const DeviceState * state = SmartThing.getDeviceState(name);
        if (state == nullptr) {
            LOGGER.error(SMART_THING_TAG, "Can't find device state with name [%s]. Not registered yet?", name);
            return -1;
        }
        LambdaCallback<String> * watcherCallback = new LambdaCallback<String>(callback, triggerValue);
        return addCallback(state, watcherCallback);
    }

    template<class T>
    int CallbacksManagerClass::addCallback(const ConfigurableObject<T> * obj, WatcherCallback<T> * callback) {
        if (obj == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Configurable object is missing, skipping...");
            return -1;
        }
        if (callback == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback is missing, skipping...");
            return -1;
        }

        Watcher<T> * watcher = getWatcherOrCreate<T>(obj);
        if (watcher == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Failed to get watcher!");
            return -1;
        }

        if (!watcher->addCallback(callback)) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Failed to add callback in watcher");
            return -1;
        }
        _callbacksCount++;
        LOGGER.info(
            CALLBACKS_MANAGER_TAG, 
            "Added new callback(id=%d) for %s [%s]",
            callback->getId(),
            obj->type,
            obj->name
        );
        return callback->getId();
    }

    template<typename T>
    Watcher<T> * CallbacksManagerClass::getWatcherOrCreate(const ConfigurableObject<T> * obj) {
        if (obj == nullptr || obj->type == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Configurable object is missing");
            return nullptr;
        }

        List<Watcher<T>> * watchersList = getWatchersList<T>();
        if (watchersList == nullptr) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Failed to define watchers list");
            return nullptr;
        }

        Watcher<T> * watcher = getWatcher<T>(watchersList, obj);
        if (watcher == nullptr) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Creating new watcher for %s [%s]", obj->type, obj->name);
            watcher = createWatcher<T>(obj);
            if (watchersList->append(watcher) < 0) {
                LOGGER.error(
                    CALLBACKS_MANAGER_TAG,
                    "Failed to append new watcher in list for %s [%s]",
                    obj->type,
                    obj->name
                );
                delete watcher;
                return nullptr;
            }
            LOGGER.info(CALLBACKS_MANAGER_TAG, "Added new watcher for %s [%s]", obj->type, obj->name);
        } else {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Watcher for %s [%s] already exists!", obj->type, obj->name);
        }
        return watcher;
    }

    template<>
    Watcher<int16_t> * CallbacksManagerClass::createWatcher(const ConfigurableObject<int16_t> * obj) {
        return new SensorWatcher((Sensor *) obj);
    }

    template<>
    Watcher<String> * CallbacksManagerClass::createWatcher(const ConfigurableObject<String> * obj) {
        return new DeviceStateWatcher((DeviceState *) obj);
    }

    template<>
    List<Watcher<int16_t>> * CallbacksManagerClass::getWatchersList() {
        return &_sensorsWatchers;
    }

    template<>
    List<Watcher<String>> * CallbacksManagerClass::getWatchersList() {
        return &_statesWatchers;
    }

    bool CallbacksManagerClass::deleteCallback(const char * type, const char * name, int id) {
        if (type == nullptr || strlen(type) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Type of observable is missing!");
            return -1;
        }

        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            return deleteWatcherCallbackFromList<int16_t>(&_sensorsWatchers, name, id);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return deleteWatcherCallbackFromList<String>(&_statesWatchers, name, id);
        }

        LOGGER.error(CALLBACKS_MANAGER_TAG, "Type [%s] not supported", type);
        return -1;
    }

    bool CallbacksManagerClass::updateCallback(DynamicJsonDocument doc) {
        JsonObject observable = doc["observable"];
        JsonObject callbackObject = doc["callback"];

        if (observable.size() == 0) {
            LOGGER.error(CALLBACK_BUILDER_TAG, "Observable object is missing!");
            return false;
        }
        if (callbackObject.size() == 0) {
            LOGGER.error(CALLBACK_BUILDER_TAG, "Callback object is missing!");
            return false;
        }
        if (!callbackObject.containsKey("id")) {
            LOGGER.error(CALLBACK_BUILDER_TAG, "Callback id property is missing!");
            return false;
        }

        const char * name = observable["name"];
        const char * type = observable["type"];
        if (name == nullptr || type == nullptr) {
            LOGGER.error(CALLBACK_BUILDER_TAG, "Observable name or type is missing!");
            return false;
        }

        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            return updateCallback<int16_t>(&_sensorsWatchers, name, callbackObject);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return updateCallback<String>(&_statesWatchers, name, callbackObject);
        } 
        LOGGER.error(CALLBACKS_MANAGER_TAG, "Observable type [%s] not supported!", type);
        return false;
    }

    template<typename T>
    bool CallbacksManagerClass::updateCallback(List<Watcher<T>> * list, const char * name, JsonObject callbackObject) {
        if (!callbackObject.containsKey("id")) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Id value in callback object is missing!");
            return false;
        }
        
        int id = callbackObject["id"];
        WatcherCallback<T> * callback = getCallbackFromWatcherList(list, name, id);
        if (callback == nullptr) {
            return false;
        }
        if (callback->isReadonly()) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Callback %d for observable [%s] is readonly!", id, name);
            return false;
        }

        if (callbackObject.containsKey("trigger")) {
            callback->setTriggerValue(callbackObject["trigger"]);
            callback->setTriggerDisabled(callbackObject["trigger"].as<String>().isEmpty());
        }
        if (callbackObject.containsKey("compareType")) {
            callback->setCompareType(callbackObject["compareType"].as<String>());
        }

        callback->updateCustom(callbackObject);
        LOGGER.info(CALLBACKS_MANAGER_TAG, "Callback id=%d for observable [%s] was updated!", id, name);
        return true;
    }

    template<typename T>
    WatcherCallback<T> * CallbacksManagerClass::getCallbackFromWatcherList(List<Watcher<T>> * list, const char * name, int id) {
        Watcher<T> * watcher = getWatcherByObservableName(list, name);
        if (watcher == nullptr) {
            LOGGER.warning(CALLBACKS_MANAGER_TAG, "Can't find watcher for observable %s", name);
            return nullptr;
        }
        WatcherCallback<T> * callback = watcher->getCallbackById(id);
        if (callback == nullptr) {
            LOGGER.warning(CALLBACKS_MANAGER_TAG, "Can't find callback id=%d for observable [%s]", id, name);
            return nullptr;
        }
        return callback;
    }

    template<typename T>
    bool CallbacksManagerClass::deleteWatcherCallbackFromList(List<Watcher<T>> * list, const char * name, int id) {
        if (name == nullptr || strlen(name) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Name of observable is missing!");
            return false;
        }
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Trying to delete observable [%s]'s callback id=%d", name, id);
        Watcher<T> * watcher = getWatcherByObservableName(list, name);
        if (watcher == nullptr || !watcher->removeCallback(id)) {
            return false;
        }
        _callbacksCount--;
        LOGGER.warning(CALLBACKS_MANAGER_TAG, "Callback № %d of observable [%s] was deleted", id, name);
        if (watcher->haveCallbacks()) {
            return true;
        }
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "No callbacks left for observable [%s], removing watcher!", name);
        if (!list->remove(watcher)) {
            return false;
        }
        delete watcher;
        LOGGER.warning(CALLBACKS_MANAGER_TAG, "Watcher for observable [%s] removed!", name);
        return true;
    }

    DynamicJsonDocument CallbacksManagerClass::getWatchersInfo() {
        DynamicJsonDocument doc(1024);
        if (_sensorsWatchers.size() > 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Collecting info from sensors watchers");
            JsonArray array = doc.createNestedArray(SENSOR_WATCHER_TYPE);
            collectInfo<int16_t>(&_sensorsWatchers, &array);
        }
        if (_statesWatchers.size() > 0) {
            LOGGER.debug(CALLBACKS_MANAGER_TAG, "Collecting info from device state watchers");
            JsonArray array = doc.createNestedArray(STATE_WATCHER_TYPE);
            collectInfo<String>(&_statesWatchers, &array);
        }
        return doc;
    }

    DynamicJsonDocument CallbacksManagerClass::getObservableCallbacksJson(const char * type, const char * name) {
        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            return getObservableCallbacksJsonFromList<int16_t>(&_sensorsWatchers, name);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return getObservableCallbacksJsonFromList<String>(&_statesWatchers, name);

        }
        LOGGER.error(CALLBACKS_MANAGER_TAG, "Type [%s] not supported", type);
        DynamicJsonDocument doc(4);
        return doc;
    }

    template<typename T>
    DynamicJsonDocument CallbacksManagerClass::getObservableCallbacksJsonFromList(List<Watcher<T>> * list, const char * name) {
        if (name == nullptr || strlen(name) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Name of observable is missing!");
        } else {
            Watcher<T> * watcher = getWatcherByObservableName(list, name);
            if (watcher != nullptr) {
                return watcher->getObservableCallbacksJson();
            } else {
                LOGGER.warning(CALLBACKS_MANAGER_TAG, "Can't find watcher for observable [%s]", name);
            }
        }
        DynamicJsonDocument doc(4);
        return doc;
    }

    DynamicJsonDocument CallbacksManagerClass::getCallbackJsonById(const char * type, const char * name, int id) {
        if (strcmp(type, SENSOR_WATCHER_TYPE) == 0) {
            return getCallbackJsonFromList<int16_t>(&_sensorsWatchers, name, id);
        } else if (strcmp(type, STATE_WATCHER_TYPE) == 0) {
            return getCallbackJsonFromList<String>(&_statesWatchers, name, id);

        }
        LOGGER.error(CALLBACKS_MANAGER_TAG, "Type [%s] not supported", type);
        DynamicJsonDocument doc(4);
        return doc;
    }

    template<typename T>
    DynamicJsonDocument CallbacksManagerClass::getCallbackJsonFromList(List<Watcher<T>> * list, const char * name, int id) {
        if (name == nullptr || strlen(name) == 0) {
            LOGGER.error(CALLBACKS_MANAGER_TAG, "Name of observable is missing!");
        } else {
            Watcher<T> * watcher = getWatcherByObservableName(list, name);
            if (watcher != nullptr) {
                WatcherCallback<T> * callback = watcher->getCallbackById(id);
                if (callback != nullptr) {
                    return callback->toJson(false);
                } else {
                    LOGGER.warning(CALLBACKS_MANAGER_TAG, "Can't find callback with id [%d] in watcher for [%s]", id, name);
                }
            } else {
                LOGGER.warning(CALLBACKS_MANAGER_TAG, "Can't find watcher for observable [%s]", name);
            }
        }
        DynamicJsonDocument doc(4);
        return doc;
    }

    template<typename T>
    Watcher<T> * CallbacksManagerClass::getWatcher(List<Watcher<T>> * list, const ConfigurableObject<T> * observable) {
        return list->findValue([observable](Watcher<T> * current) {
            return current->getObservable() == observable;
        });
    }

    template<typename T>
    Watcher<T> * CallbacksManagerClass::getWatcherByObservableName(List<Watcher<T>> * list, const char * name) {
        return list->findValue([name](Watcher<T> * current) {
            return strcmp(current->getObservable()->name, name) == 0;
        });
    }

    template<typename T>
    void CallbacksManagerClass::collectInfo(List<Watcher<T>> * list, JsonArray * array) {
        list->forEach([&](Watcher<T> * current) {
            array->add(current->getObservableInfo());
        });
    }

    void CallbacksManagerClass::check() {
        if (_sensorsWatchers.size() > 0) {
            checkWatchers<int16_t>(&_sensorsWatchers);
        }
        if (_statesWatchers.size() > 0) {
            checkWatchers<String>(&_statesWatchers);
        }
    }

    template<typename T>
    void CallbacksManagerClass::checkWatchers(List<Watcher<T>> * list) {
        list->forEach([](Watcher<T> * current) {
            current->check();
        });
    }

    void CallbacksManagerClass::saveCallbacksToSettings() {
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Saving callbacks");
        STSettings.setCallbacks(allCallbacksToJson(true, true).as<JsonArray>());
        STSettings.save();
        LOGGER.debug(CALLBACKS_MANAGER_TAG, "Callbacks were saved");
    }

    DynamicJsonDocument CallbacksManagerClass::getCallbacksTemplates() {
        DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE * 2);
        //<bruh>
        doc["default"] = WatcherCallback<int>::getTemplate();
        doc[HTTP_CALLBACK_TAG] = HttpCallback<int>::getTemplate();
        doc[ACTION_CALLBACK_TAG] = ActionCallback<int>::getTemplate();
        return doc;
    }
}