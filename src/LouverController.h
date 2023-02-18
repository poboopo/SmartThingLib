#include <Arduino.h>
#include <mech/MotorController.h>
#include <utils/LedIndicator.h>

#define LIGHT_MONITOR_TAG "light_monitor"

#define MONITOR_TASK_DELAY 500

#define CLOSE_POSITION 0
#define MIDDLE_POSITION 1200
#define OPEN_POSITION 2500
#define BRIGHT_POSITION 4095

class LouverController {
    private:
        TaskHandle_t _monitorLightHandle = NULL;
        MotorController _motorController;
        LedIndicator * _led = NULL;

        uint8_t _lightSensorPin;

        void createMonitorTask();
        void deleteMonitorTask();
        void monitorLight();
    public:
        LouverController();
        ~LouverController();
        // pass ledPin < 0 if u don't wanna led indication
        void init(uint8_t motorFirstPin,
                         uint8_t motorSecondPin,
                         uint8_t potPin,
                         uint8_t lightSensorPin);
        void addLedIndicator(LedIndicator * led);

        void enableAutoMode();
        void disabelAutoMode();
        bool isAutoModeEnabled();
        uint16_t getLightValue();
        uint16_t getMotorPosition();

        void open();
        void close();
        void middle();
        void bright();
};
