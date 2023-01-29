#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <LouverController.h>

#include <net/Multicaster.h>
#include <net/WebUtils.h>
#include <utils/SettingsManager.h>

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35
#define LED_PIN 5
#define BUTTON_PIN 18

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

LouverController controller;
Multicaster multicaster;
SettingsManager settingsManager;
WebServer server(80);
const char * myIp;

void setupServerEndPoints();

void setup() {
    ESP_LOGI("*", "Setup started");
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    if (!digitalRead(BUTTON_PIN)) {
        settingsManager.clear();
    }

    settingsManager.loadSettings();
    ESP_LOGI("*", "Settings manager loaded");

    String ssid = settingsManager.getSetting(SSID_SETTING);
    String password = settingsManager.getSetting(PASSWORD_SETTING);
    
    ESP_LOGI("*", "WiFi connecting to %s :: %s", ssid.c_str(), password.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    while (!WiFi.isConnected()) {
    }
    ESP_LOGI("*", "WiFi connected %s", WiFi.localIP());

    controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN, LED_PIN);
    ESP_LOGI("*", "Controller created");
    
    ArduinoOTA.begin();
    ESP_LOGI("*", "Ota started");

    multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
    ESP_LOGI("*", "Multicaster created");

    setupServerEndPoints();
    server.begin();
    ESP_LOGI("*", "Web server endpoints configured and started");

    myIp = WiFi.localIP().toString().c_str();

    ESP_LOGI("*", "Setup finished");
}

void loop() {
    ArduinoOTA.handle();
    server.handleClient();

    if (!digitalRead(BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disabelAutoMode();
        } else {
            controller.enableAutoMode();
        }
        ESP_LOGI("*", "Free heap: %u", ESP.getFreeHeap());
    }

    multicaster.broadcast(myIp);
    delay(500);
}

void setupServerEndPoints() {
    server.on("/", []() {
        server.send(200, "text/html", greetingPage);
    });
    server.on("/louver", HTTP_GET, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[GET] [/louver]");
        handleLouverGet(&server, &controller);
    });
    server.on("/louver", HTTP_PUT, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[PUT] [/louver]");
        handleLouverPut(&server, &controller);
    });

    server.on("/settings", HTTP_GET, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[GET] [/settings]");
        server.send(200, "application/json", settingsManager.getJson());
    });
    server.on("/settings", HTTP_POST, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[POST] [/settings]");
        if (!server.hasArg("plain")) {
            server.send(400, "content/json", buildErrorJson("Body is missing"));
            return;
        }
        String data = server.arg("plain");
        if (data.length() == 0) {
            server.send(400, "content/json", buildErrorJson("Body is missing"));
            return;
        }

        Dictionary &dict = *(new Dictionary());
        dict.jload(data);
        if (dict.size() == 0) {
            server.send(400, "content/json", buildErrorJson("Body is missing"));
            return;
        }

        settingsManager.mergeSettings(dict);
        settingsManager.saveSettings();

        server.send(200);
    });
    server.on("/settings", HTTP_DELETE, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[DELETE] [/settings]");
        if (!server.hasArg("name")) {
            server.send(400, "content/json", buildErrorJson("Setting name is missing"));
        }

        settingsManager.removeSetting(server.arg("name"));
        settingsManager.saveSettings();
        server.send(200);
    });
    server.onNotFound([](){
        server.send(404, "application/json", buildErrorJson("Page not found"));
    });
}