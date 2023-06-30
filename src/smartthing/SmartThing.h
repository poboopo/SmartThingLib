#ifndef SMART_THING_H
#define SMART_THING_H

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "smartthing/settings/SettingsManager.h"
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/net/socket/Multicaster.h"
#include "smartthing/net/rest/RestController.h"
#include "smartthing/utils/LedIndicator.h"

#define SMART_THING_VERSION 0.1
#define SMART_THING_TAG "SMART_THING"
// Pins
#define LED_PIN 5
#define BUTTON_PIN 18

#define WIFI_SETUP_TIMEOUT 10000
#define WIPE_BUTTON_TIME 5000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

#define DEVICE_NAME_LENGTH_MAX 15


class SmartThingClass {
    public:
        ~SmartThingClass();
        SmartThingClass();

        bool init(String type);
        void loopRoutine();
        void setName(String name); // should save name to config
        const String getType();
        const String getName();
        bool wifiConnected();

        RestController* getRestController();
        LedIndicator* getLed(); // are u sure u need this?
    private:
        Multicaster _multicaster;
        RestController _rest;
        LedIndicator _led;

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