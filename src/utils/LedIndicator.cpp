#include "utils/LedIndicator.h"

LedIndicator::LedIndicator() {}

LedIndicator::~LedIndicator() { off(); }

void LedIndicator::init(uint8_t ledPin) {
  _ledPin = ledPin;
  pinMode(_ledPin, OUTPUT);
  off();
}

void LedIndicator::off() {
  if (_taskHandle != NULL) {
    vTaskDelete(_taskHandle);
    _taskHandle = NULL;
  }
  digitalWrite(_ledPin, LOW);
  _enabled = false;
}

void LedIndicator::on() {
  if (_taskHandle != NULL) {
    vTaskDelete(_taskHandle);
    _taskHandle = NULL;
  }
  digitalWrite(_ledPin, HIGH);
  _enabled = true;
}

void LedIndicator::blinkTask() {
  bool state = true;
  const TickType_t xDelay = BLINK_DELAY / portTICK_PERIOD_MS;

  _enabled = true;
  while (_count == -1 || _count > 0) {
    digitalWrite(_ledPin, state);
    vTaskDelay(xDelay);
    state = !state;
    if (_count != -1) {
      _count--;
    }
  }
  _enabled = false;

  vTaskDelete(_taskHandle);
}

void LedIndicator::blink() { blink(-1); }

void LedIndicator::blink(int8_t count) {
  off();
  _count = count;
  xTaskCreate([](void* o) { static_cast<LedIndicator*>(o)->blinkTask(); },
              LED_INDICATOR_TAG, 2048, this, 1, &_taskHandle);
}
