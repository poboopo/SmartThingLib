#include <LouverController.h>

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
    int16_t delayTime = _taskDelay / portTICK_PERIOD_MS;

    ESP_LOGI(LIGHT_MONITOR_TAG, "Light monitor task started");
    for(;;) {
        lightValue = analogRead(_lightSensorPin);
        if (lightValue > _lightClose) {
            _motorController.setPosition(CLOSE_POSITION);
        } else if (lightValue < _lightBright) {
            _motorController.setPosition(BRIGHT_POSITION);
        } else if (lightValue < _lightOpen && lightValue > _lightBright) {
            _motorController.setPosition(map(lightValue, _lightBright, _lightOpen, BRIGHT_POSITION, OPEN_POSITION));
        } else if (lightValue < _lightClose && lightValue > _lightOpen) {
            _motorController.setPosition(map(lightValue, _lightOpen, _lightClose, OPEN_POSITION, CLOSE_POSITION));
        }
        vTaskDelay(delayTime);
    }
}

void LouverController::createMonitorTask(){
    xTaskCreate(
        [](void* o){ static_cast<LouverController*>(o)->monitorLight(); },
        LIGHT_MONITOR_TAG,
        2048,
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
    if (disabelAutoMode()) {
        delay(100);
        enableAutoMode();
    }
}

void LouverController::enableAutoMode() {
    if (!isAutoModeEnabled()) {
        createMonitorTask();
        if (_led != NULL) {
            _led->on();
        }
    }
    ESP_LOGI(LOUVER_CONTROLLER_TAG, "Automode enabled");
}

bool LouverController::disabelAutoMode() {
    if (isAutoModeEnabled()) {
        deleteMonitorTask();
        if (_led != NULL) {
            _led->off();
        }
        ESP_LOGI(LOUVER_CONTROLLER_TAG, "Automode disabled");
        return true;
    }
    return false;
}

bool LouverController::isAutoModeEnabled() {
    return _monitorLightHandle != NULL;
}

uint16_t LouverController::getLightValue() {
    return analogRead(_lightSensorPin);
}

uint16_t LouverController::getMotorPosition() {
    return _motorController.currentPosition();
}

void LouverController::open() {
    disabelAutoMode();
    _motorController.setPosition(OPEN_POSITION);
}

void LouverController::close() {
    disabelAutoMode();
    _motorController.setPosition(CLOSE_POSITION);
}

void LouverController::middle() {
    disabelAutoMode();
    _motorController.setPosition(MIDDLE_POSITION);
}

void LouverController::bright() {
    disabelAutoMode();
    _motorController.setPosition(BRIGHT_POSITION);
}