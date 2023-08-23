#include "LouverController.h"

#define LIGHT_AVERAGE_OF 10
#define LIGHT_FLOATING_AVERAGE_OF 5
#define LIGHT_THRESHOLD 50

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

int16_t calculateFloatingAverage(int16_t * floating, int16_t newValue) {
    int16_t res = 0;
    for (int i = 1; i < LIGHT_FLOATING_AVERAGE_OF; i++) {
        *(floating + i - 1) = *(floating + i);
        res += *(floating + i);
    }
    *(floating + LIGHT_FLOATING_AVERAGE_OF - 1) = newValue;
    res += newValue;
    if (res == 0) {
        return 0;
    }
    return res / LIGHT_FLOATING_AVERAGE_OF;
}

void LouverController::monitorLight() {
    int16_t lightValue = getLightValue();
    int16_t oldLightValue = -1;
    int16_t floating[LIGHT_FLOATING_AVERAGE_OF]; // плавующее среднее для значения датчика света
    const TickType_t xDelay = _taskDelay / portTICK_PERIOD_MS;

    for (int8_t i = 0; i < LIGHT_FLOATING_AVERAGE_OF; i++) {
        floating[i] = lightValue;
    }

    // TODO реализовать некую функцию, которая заменит это все
    // Кривая фурье?
    for(;;) {
        lightValue = calculateFloatingAverage(floating, getLightValue());
        if (abs(lightValue - oldLightValue) > LIGHT_THRESHOLD || oldLightValue == -1) {
            if (lightValue < _lightClose) {
                _motorController.setPosition(CLOSE_POSITION);
            } else if (lightValue > _lightBright) {
                _motorController.setPosition(BRIGHT_POSITION);
            } else if (lightValue > _lightOpen && lightValue < _lightBright) {
                _motorController.setPosition(map(lightValue, _lightBright, _lightOpen, BRIGHT_POSITION, OPEN_POSITION));
            } else if (lightValue > _lightClose && lightValue < _lightOpen) {
                _motorController.setPosition(map(lightValue, _lightOpen, _lightClose, OPEN_POSITION, CLOSE_POSITION));
            } else {
                LOGGER.error(LOUVER_CONTROLLER_TAG, "Failed to define position for light value %d", lightValue);
            }
            oldLightValue = lightValue;
        }
        vTaskDelay(xDelay);
    }
}

const char * LouverController::getPosition() {
    uint16_t pos = _motorController.getPosition();
    if (pos <= CLOSE_POSITION && pos > MIDDLE_POSITION) {
        return "closed";
    } else if (pos <= MIDDLE_POSITION && pos > OPEN_POSITION) {
        return "middle";
    } else if (pos <= OPEN_POSITION && pos > BRIGHT_POSITION) {
        return "open";
    } else if (pos <= BRIGHT_POSITION) {
        return "bright";
    }
    return "error";
}

void LouverController::createMonitorTask(){
    xTaskCreate(
        [](void* o){ static_cast<LouverController*>(o)->monitorLight(); },
        LIGHT_MONITOR_TAG,
        6117,
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
    LOGGER.info(LOUVER_CONTROLLER_TAG, "Automode enabled");
    return true;
}

bool LouverController::disableAutoMode() {
    if (isAutoModeEnabled()) {
        deleteMonitorTask();
        if (_led != NULL) {
            _led->off();
        }
    }
    LOGGER.info(LOUVER_CONTROLLER_TAG, "Automode disabled");
    return true;
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