#include <Arduino.h>
#include <LouverController.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <net/Multicaster.h>

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35
#define LED_PIN 5
#define BUTTON_PIN 18

#define SSID "POEBLO_ROOM"
#define PASS "donttouch"

LouverController controller;
Multicaster multicaster;
String myIp;

void setup() {
    ESP_LOGI("*", "Setup started");
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    ESP_LOGI("*", "WiFi connecting to %s :: %s", SSID, PASS);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASS);
    while (!WiFi.isConnected()) {
    }
    ESP_LOGI("*", "WiFi connected %s", WiFi.localIP());

    controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN, LED_PIN);
    ESP_LOGI("*", "Controller created");
    
    ArduinoOTA.begin();
    ESP_LOGI("*", "Ota started");

    multicaster.init("224.1.1.1", 7778);
    ESP_LOGI("*", "Multicaster created");

    myIp = WiFi.localIP().toString();

    ESP_LOGI("*", "Setup finished");
}

void loop() {
    ArduinoOTA.handle();

    if (!digitalRead(BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disabelAutoMode();
        } else {
            controller.enableAutoMode();
        }
        ESP_LOGI("*", "Free heap: %u", ESP.getFreeHeap());
    }

    multicaster.broadcast(myIp.c_str());
    delay(500);
}
