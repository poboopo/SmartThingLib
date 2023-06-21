#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "utils/SettingsManager.h"
#include "net/logs/BetterLogger.h"
#include "net/socket/Multicaster.h"
#include "net/rest/RestController.h"
#include "utils/LedIndicator.h"

#define SMART_THING_VERSION 0.1

#define SMART_THING_TAG "SMART_THING"
// Pins
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

        bool init(String type);
        void loopRoutine();
        void setName(String name);
        bool wifiConnected();

        RestController* getRestController();
        SettingsManager* getSettingsManager();
        LedIndicator* getLed();
    private:
        SettingsManager _settingsManager;
        Multicaster _multicaster;
        LedIndicator _led;
        RestController _rest;

        // change to const char *?
        String _ip = "";
        String _name;
        String _type;
        String _broadcastMessage = "";

        void wipeSettings();
        String buildInfoJson();
        String connectToWifi(String, String, int);
};

#endif