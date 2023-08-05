#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "smartthing/settings/SettingsManager.h"
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/net/socket/Multicaster.h"
#include "smartthing/net/rest/RestController.h"
#include "smartthing/utils/LedIndicator.h"

#include "smartthing/configurable/SensorsList.h"
#include "smartthing/configurable/DeviceStatesList.h"
#include "smartthing/configurable/ActionsList.h"
#include "smartthing/configurable/ConfigEntriesList.h"

#include "smartthing/watcher/CallbacksManager.h"
#include "smartthing/watcher/callback/LambdaCallback.h"
#include "smartthing/watcher/callback/HttpCallback.h"

#define SMART_THING_VERSION 0.2
#define SMART_THING_TAG "SMART_THING"
// Pins
#define LED_PIN 14
#define WIPE_BUTTON_PIN 18

#define WIFI_SETUP_TIMEOUT 10000
#define WIPE_BUTTON_TIME 5000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

#define DEVICE_NAME_LENGTH_MAX 15

#define STATE_TYPE "state"
#define SENSOR_TYPE "sensor"

class SmartThingClass {
    public:
        ~SmartThingClass();
        SmartThingClass();

        bool init(String type);
        void loopRoutine();
        void setName(String name);
        const String getType();
        const String getName();
        bool wifiConnected();

        bool registerSensor(const char * name, Configurable::ConfigurableObject<int16_t>::ValueGeneratorFunction valueGenerator);
        bool registerDigitalSensor(const char * name, int pin);
        bool registerAnalogSensor(const char * name, int pin);
        bool addDeviceState(const char * name, Configurable::ConfigurableObject<const char *>::ValueGeneratorFunction valueGenerator);
        bool addActionHandler(const char * action, const char * caption, Configurable::Action::ActionHandler handler);
        bool addActionHandler(const char * action, Configurable::Action::ActionHandler handler) {
            return addActionHandler(action, action, handler);
        };
        Configurable::Action::ActionResult callAction(const char * action);
        bool addConfigEntry(const char * name, const char * caption, const char * type);

        bool addDeviceStateCallback(const char * name, Callback::LambdaCallback<char *>::CustomCallback callback, const char * triggerValue);
        bool addDeviceStateCallback(const char * name, Callback::LambdaCallback<char *>::CustomCallback callback) {
            return addDeviceStateCallback(name, callback, nullptr);
        };
        bool addDeviceStateCallback(const char * name, const char * url, const char * triggerValue, bool readonly);
        bool addDeviceStateCallback(const char * name, const char * url, const char * triggerValue) {
            return addDeviceStateCallback(name, url, triggerValue, true);
        }
        bool addDeviceStateCallback(const char * name, const char * url) {
            return addDeviceStateCallback(name, url, nullptr, true);
        };

        bool addSensorCallback(const char * name, Callback::LambdaCallback<int16_t>::CustomCallback callback, int16_t triggerValue);
        bool addSensorCallback(const char * name, Callback::LambdaCallback<int16_t>::CustomCallback callback) {
            return addSensorCallback(name, callback, -1);
        };
        bool addSensorCallback(const char * name, const char * url, int16_t triggerValue, bool readonly);
        bool addSensorCallback(const char * name, const char * url, int16_t triggerValue) {
            return addSensorCallback(name, url, triggerValue, true);
        }
        bool addSensorCallback(const char * name, const char * url) {
            return addSensorCallback(name, url, -1, true);
        };

        bool createCallbacksFromJson(const char * body);
        bool deleteCallback(const char * type, const char * name, int16_t index) {
            return _callbacksManager.deleteCallback(type, name, index);
        }
        bool updateCallback(const char * type, const char * name, int16_t index, const char * json) {
            return _callbacksManager.updateCallback(type, name, index, json);
        }

        DynamicJsonDocument getInfoionaries();
        DynamicJsonDocument getSensorsValues();
        DynamicJsonDocument getDeviceStatesInfo();
        DynamicJsonDocument getActionsInfo();
        DynamicJsonDocument getConfigEntriesInfo();
        DynamicJsonDocument getWatchersInfo();
        DynamicJsonDocument getCallbacksJson(const char * watcherType, const char * name);

        RestController* getRestController();
        LedIndicator* getLed(); // are u sure u need this?
    private:
        Multicaster _multicaster;
        RestController _rest;
        LedIndicator _led;
        Configurable::Sensor::SensorsList _sensorsList;
        Configurable::DeviceState::DeviceStatesList _deviceStatesList;
        Configurable::Action::ActionsList _actionsList;
        Configurable::Config::ConfigEntriesList _configEntriesList;
        Callback::CallbacksManager _callbacksManager;

        // todo change to const * char?
        String _ip;
        String _name;
        String _type;
        String _broadcastMessage;

        void wipeSettings();
        String connectToWifi();
};

extern SmartThingClass SmartThing;

#endif