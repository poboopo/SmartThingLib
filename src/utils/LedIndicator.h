#include <Arduino.h>

#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

// todo remove
#define LED_INDICATOR_TAG "led_indicator"
#define BLINK_DELAY 200

class LedIndicator {
 private:
  uint8_t _ledPin;
  bool _enabled;

 public:
  LedIndicator() {};
  ~LedIndicator() {
    off();
  };

  void init(uint8_t ledPin)  {
    _ledPin = ledPin;
    pinMode(_ledPin, OUTPUT);
    off();
  }

  void on() {
    digitalWrite(_ledPin, HIGH);
    _enabled = true;
  }

  void off() {
    digitalWrite(_ledPin, LOW);
    _enabled = false;
  }

  bool isOn() {
    return _enabled;
  }
};

#endif