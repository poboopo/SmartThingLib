#include <Arduino.h>
#include <MotorController.h>

#define LIGHT_MONITOR_TAG "light_monitor"

#define MONITOR_TASK_DELAY 500

struct TaskData {
    MotorController * controller;
    uint8_t sensorPin;
};

class LouverController {
    private:
        TaskHandle_t _monitorLightHandle = NULL;
        MotorController _motorController;
        TaskData _taskdata;

        uint8_t _lightSensorPin;
        int8_t _ledPin = -1;

        void createMonitorTask();
        void deleteMonitorTask();
        void changeLedState(bool);
    public:
        LouverController();
        ~LouverController();
        // pass ledPin < 0 if u don't wanna led indication
        void init(uint8_t motorFirstPin,
                         uint8_t motorSecondPin,
                         uint8_t potPin,
                         uint8_t lightSensorPin,
                         int8_t ledPin);

        void enableAutoMode();
        void disabelAutoMode();
        bool isAutoModeEnabled();
};
