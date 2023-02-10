#include <Arduino.h>

#define LED_INDICATOR_TAG "led_indicator"
#define BLINK_DELAY 200

class LedIndicator {
    private:
        uint8_t _ledPin;
        int8_t _count;
        TaskHandle_t _taskHandle = NULL;
        void blinkTask();
    public:
        LedIndicator();
        ~LedIndicator();
        void init(uint8_t ledPin);
        void blink();
        void blink(int8_t count);
        void on();
        void off();
};