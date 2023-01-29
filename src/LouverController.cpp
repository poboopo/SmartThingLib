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
                                    uint8_t lightSensorPin,
                                    int8_t ledPin) {
    _motorController = MotorController(motorFirstPin, motorSecondPin, potPin);
    _lightSensorPin = lightSensorPin;
    if (ledPin >= 0) {
        _ledPin = ledPin;
        pinMode(_ledPin, OUTPUT);
    }
}

void monitorLight(void * param) {
    TaskData * taskData = (TaskData*) param;

    int16_t lightValue = 0;
    int16_t delayTime = MONITOR_TASK_DELAY / portTICK_PERIOD_MS;

    ESP_LOGI(LIGHT_MONITOR_TAG, "Light monitor task started");
    for(;;) {
        lightValue = map(analogRead(taskData->sensorPin), 0, 4095, POT_MIN, POT_MAX);
        taskData->controller->setPosition(lightValue);
        vTaskDelay(delayTime);
    }
}

void LouverController::createMonitorTask(){
    _taskdata.controller = &_motorController;
    _taskdata.sensorPin = _lightSensorPin;
    xTaskCreate(
        monitorLight,
        LIGHT_MONITOR_TAG,
        2048,
        (void*) &_taskdata,
        1,
        &_monitorLightHandle
    );
}

void LouverController::deleteMonitorTask() {
    if (_monitorLightHandle != NULL) {
        vTaskDelete(_monitorLightHandle);
        _monitorLightHandle = NULL;
    }
}

void LouverController::changeLedState(bool state) {
    if (_ledPin >= 0) {
        digitalWrite(_ledPin, state);
    } 
}

void LouverController::enableAutoMode() {
    if (!isAutoModeEnabled()) {
        createMonitorTask();
        changeLedState(true);
    }
    ESP_LOGI(LOUVER_CONTROLLER_TAG, "Automode enabled");
}

void LouverController::disabelAutoMode() {
    if (isAutoModeEnabled()) {
        deleteMonitorTask();
        changeLedState(false);
    }
    ESP_LOGI(LOUVER_CONTROLLER_TAG, "Automode disabled");
}

bool LouverController::isAutoModeEnabled() {
    return _monitorLightHandle != NULL;
}