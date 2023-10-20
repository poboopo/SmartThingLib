#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "settings/SettingsManager.h"
#include "logs/BetterLogger.h"
#include "net/socket/Multicaster.h"
#include "net/rest/RestController.h"
#include "utils/LedIndicator.h"

#include "configurable/SensorsList.h"
#include "configurable/DeviceStatesList.h"
#include "configurable/ActionsList.h"
#include "configurable/ConfigEntriesList.h"

#include "watcher/CallbacksManager.h"

#define SMART_THING_VERSION 0.4
#define SMART_THING_TAG "SMART_THING"
// Pins
#define LED_PIN 14
#define WIPE_BUTTON_PIN 18

#define WIFI_SETUP_TIMEOUT 10000
#define WIPE_BUTTON_TIME 5000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

#define DEVICE_NAME_LENGTH_MAX 15
#define SMART_THING_LOOP_TASK_DELAY 250 //ms

class SmartThingClass {
    public:
        ~SmartThingClass();
        SmartThingClass();

        bool init(String type);
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

        const Configurable::DeviceState::DeviceState * getDeviceState(const char * name);
        const Configurable::Sensor::Sensor * getSensor(const char * name);

        DynamicJsonDocument getInfoDictionaries();
        DynamicJsonDocument getSensorsValues();
        DynamicJsonDocument getDeviceStatesInfo();
        DynamicJsonDocument getActionsInfo();
        DynamicJsonDocument getConfigInfo();
        DynamicJsonDocument getWatchersInfo();

        int16_t getSensorsCount() {
            return _sensorsList.size();
        }

        int16_t getDeviceStatesCount() {
            return _deviceStatesList.size();
        }

        RestController* getRestController() {
            return &_rest;
        }
        Callback::CallbacksManager * getCallbacksManager() {
            return &_callbacksManager;
        }
        LedIndicator* getLed() {
            return &_led;
        }
    private:
        Multicaster _multicaster;
        RestController _rest;
        LedIndicator _led;
        Configurable::Sensor::SensorsList _sensorsList;
        Configurable::DeviceState::DeviceStatesList _deviceStatesList;
        Configurable::Action::ActionsList _actionsList;
        Configurable::Config::ConfigEntriesList _configEntriesList;
        Callback::CallbacksManager _callbacksManager;
        TaskHandle_t _loopTaskHandle = NULL;
        void updateBroadCastMessage();

        // todo change to const * char?
        String _ip;
        String _name;
        String _type;
        String _broadcastMessage;

        void wipeSettings();
        String connectToWifi();

        void loopRoutine();
};

extern SmartThingClass SmartThing;

#endif