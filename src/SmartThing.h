#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "utils/SettingsManager.h"
#include "net/BetterLogger.h"
#include "net/Multicaster.h"
#include "net/RestController.h"
#include "utils/LedIndicator.h"

#define SMART_THING_VERSION 1

#define SMART_THING_TAG "SMART_THING"
// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35
#define LED_PIN 5
#define BUTTON_PIN 18

#define WIFI_SETUP_TIMEOUT 10000
#define WIPE_BUTTON_TIME 5000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

class SmartThing {
    public:
        SmartThing();
        ~SmartThing();

        bool init();
        void loopRoutine();
        void setName(String name);

        RestController* getRestController();
        SettingsManager* getSettingsManager();
        LedIndicator* getLed();
    private:
        SettingsManager _settingsManager;
        Multicaster _multicaster;
        LedIndicator _led;
        RestController _rest;

        String _ip = "";
        String _name;
        String _broadcastMessage = "";

        void wipeSettings();
        String buildInfoJson();
        String connectToWifi(String ssid, String password);
};

#endif