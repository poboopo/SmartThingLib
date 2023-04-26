#include "LouverController.h"

#define LIGHT_AVERAGE_OF 20

LouverController::LouverController() {
}

LouverController::~LouverController() {
    _motorController.stop();
    deleteMonitorTask();
}

void LouverController::init( uint8_t motorFirstPin,
                                    uint8_t motorSecondPin,
                                    uint8_t potPin,
                                    uint8_t lightSensorPin) {
    _motorController = MotorController(motorFirstPin, motorSecondPin, potPin);
    _lightSensorPin = lightSensorPin;
}

void LouverController::setMotorAccuracy(uint16_t accuracy) {
    _motorController.setAccuracy(accuracy);
}

void LouverController::addLedIndicator(LedIndicator * led) {
    _led = led;
}

void LouverController::setLightValues(
                                    uint16_t lightClose,
                                    uint16_t lightOpen,
                                    uint16_t lightBright) {
    if (lightClose == _lightClose && lightOpen == _lightOpen && lightBright == _lightBright) {
        return;
    }

    _lightClose = lightClose;
    _lightOpen = lightOpen;
    _lightBright = lightBright;
}

void LouverController::setMonitorTaskDelay(uint16_t delay) {
    _taskDelay = delay;
}

void LouverController::monitorLight() {
    int16_t lightValue = 0;
    const TickType_t xDelay = _taskDelay / portTICK_PERIOD_MS;

    // TODO реализовать некую функцию, которая заменит это все
    // Кривая фурье?
    for(;;) {
        lightValue = getLightValue();
        if (lightValue < _lightClose) {
            _motorController.setPosition(CLOSE_POSITION);
        } else if (lightValue > _lightBright) {
            _motorController.setPosition(BRIGHT_POSITION);
        } else if (lightValue > _lightOpen && lightValue < _lightBright) {
            _motorController.setPosition(map(lightValue, _lightBright, _lightOpen, BRIGHT_POSITION, OPEN_POSITION));
        } else if (lightValue > _lightClose && lightValue < _lightOpen) {
            _motorController.setPosition(map(lightValue, _lightOpen, _lightClose, OPEN_POSITION, CLOSE_POSITION));
        }
        vTaskDelay(xDelay);
    }
}

void LouverController::createMonitorTask(){
    xTaskCreate(
        [](void* o){ static_cast<LouverController*>(o)->monitorLight(); },
        LIGHT_MONITOR_TAG,
        3072,
        this,
        1,
        &_monitorLightHandle
    );
}

bool LouverController::deleteMonitorTask() {
    if (_monitorLightHandle != NULL) {
        vTaskDelete(_monitorLightHandle);
        _monitorLightHandle = NULL;
        return true;
    }
    return false;
}

void LouverController::restartAutoMode() {
    if (disableAutoMode()) {
        delay(100);
        enableAutoMode();
    }
}

bool LouverController::enableAutoMode() {
    if (!isAutoModeEnabled()) {
        createMonitorTask();
        if (_led != NULL) {
            _led->on();
        }
    }
    BetterLogger::log(LOUVER_CONTROLLER_TAG, "Automode enabled");
    return true;
}

bool LouverController::disableAutoMode() {
    if (isAutoModeEnabled()) {
        deleteMonitorTask();
        if (_led != NULL) {
            _led->off();
        }

        BetterLogger::log(LOUVER_CONTROLLER_TAG, "Automode disabled");
        return true;
    }
    return false;
}

bool LouverController::isAutoModeEnabled() {
    return _monitorLightHandle != NULL;
}

uint16_t LouverController::getLightValue() {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < LIGHT_AVERAGE_OF; i++) {
        sum += analogRead(_lightSensorPin);
    }
    return sum / LIGHT_AVERAGE_OF;
}

uint16_t LouverController::getMotorPosition() {
    return _motorController.getPosition();
}

bool LouverController::open() {
    disableAutoMode();
    return _motorController.setPosition(OPEN_POSITION);
}

bool LouverController::close() {
    disableAutoMode();
    return _motorController.setPosition(CLOSE_POSITION);
}

bool LouverController::middle() {
    disableAutoMode();
    return _motorController.setPosition(MIDDLE_POSITION);
}

bool LouverController::bright() {
    disableAutoMode();
    return _motorController.setPosition(BRIGHT_POSITION);
}