#include <Arduino.h>
#include "mech/MotorController.h"
#include "utils/LedIndicator.h"
#include "net/BetterLogger.h"

#ifndef LouverController_H
#define LouverController_H

#define LIGHT_MONITOR_TAG "light_monitor"
#define LOUVER_CONTROLLER_TAG "louver_controller"

#define MONITOR_TASK_DELAY 500

#define CLOSE_POSITION 4095
#define MIDDLE_POSITION 2500
#define OPEN_POSITION 1200
#define BRIGHT_POSITION 0

#define LIGHT_CLOSE 1000
#define LIGHT_OPEN 200
#define LIGHT_BRIGHT 50

class LouverController {
    private:
        TaskHandle_t _monitorLightHandle = NULL;
        MotorController _motorController;
        LedIndicator * _led = NULL;
        BetterLogger * _logger = NULL;

        uint16_t _lightClose = LIGHT_CLOSE;
        uint16_t _lightOpen = LIGHT_OPEN;
        uint16_t _lightBright = LIGHT_BRIGHT;
        uint16_t _taskDelay = MONITOR_TASK_DELAY;

        uint8_t _lightSensorPin;

        void createMonitorTask();
        bool deleteMonitorTask();
        void monitorLight();
    public:
        LouverController();
        ~LouverController();

        void init(uint8_t motorFirstPin,
                         uint8_t motorSecondPin,
                         uint8_t potPin,
                         uint8_t lightSensorPin);

        void addLogger(BetterLogger * logger);
        void addLedIndicator(LedIndicator * led);
        void setLightValues(
            uint16_t lightClose,
            uint16_t lightOpen,
            uint16_t lightBright
        );
        void setMonitorTaskDelay(uint16_t delay);

        void enableAutoMode();
        bool disableAutoMode();
        void restartAutoMode();

        bool isAutoModeEnabled();
        uint16_t getLightValue();
        uint16_t getMotorPosition();

        void open();
        void close();
        void middle();
        void bright();
};
#endif