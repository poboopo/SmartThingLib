#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include <LouverController.h>
#include <net/Multicaster.h>
#include <net/WebUtils.h>

// Pins
#define MOTOR_FIRST_PIN 26
#define MOTOR_SECOND_PIN 25
#define POT_PIN 36
#define LIGHT_SENSOR_PIN 35
#define LED_PIN 5
#define BUTTON_PIN 18

#define WIFI_SETUP_TIMEOUT 10000
#define WIPE_BUTTON_TIME 5000

#define MULTICAST_GROUP "224.1.1.1"
#define MULTICAST_PORT 7778

LouverController controller;
Multicaster multicaster;
SettingsManager settingsManager;
WebServer server(80);
LedIndicator ledIndicator;

String myIp;

void setupServerEndPoints();
String connectToWifi();
void wipeSettings();
void processConfig();

bool wifiConnected() {
    return WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP;
}

void setup() {
    ESP_LOGI("*", "Setup started");
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    ledIndicator.init(LED_PIN);
    settingsManager.loadSettings();
    ESP_LOGI("*", "Settings manager loaded");
    
    if (!digitalRead(BUTTON_PIN)) {
        wipeSettings();
    }

    myIp = connectToWifi();

    if (wifiConnected()) {
        ESP_LOGI("*", "WiFi connected, local ip %s", myIp);

        ArduinoOTA.begin();
        ESP_LOGI("*", "Ota started");

        multicaster.init(MULTICAST_GROUP, MULTICAST_PORT);
        ESP_LOGI("*", "Multicaster created");

        setupServerEndPoints();
        server.begin();
        ESP_LOGI("*", "Web server endpoints configured and started");
    } else {
        ESP_LOGI("*", "WiFi not available, skipping all network setup");
    }

    controller.init(MOTOR_FIRST_PIN, MOTOR_SECOND_PIN, POT_PIN, LIGHT_SENSOR_PIN);
    controller.addLedIndicator(&ledIndicator);
    if (settingsManager.getSettingInteger(GROUP_STATE, AUTOMODE_SETTING)) {
        controller.enableAutoMode();
    }
    ESP_LOGI("*", "Controller created");

    processConfig();
    ESP_LOGI("*", "Config proceed");

    ESP_LOGI("*", "Setup finished");
}

void loop() {
    if (wifiConnected()) {
        ArduinoOTA.handle();
        server.handleClient();
        multicaster.broadcast(myIp.c_str());
    }

    if (!digitalRead(BUTTON_PIN)) {
        if (controller.isAutoModeEnabled()) {
            controller.disabelAutoMode();
        } else {
            controller.enableAutoMode();
        }
        ESP_LOGI("*", "Free heap: %u", ESP.getFreeHeap());
    }

    delay(500);
}

void wipeSettings() {
    long started = millis();
    ledIndicator.on();
    while (!digitalRead(BUTTON_PIN) && millis() - started < WIPE_BUTTON_TIME) {
    }
    ledIndicator.off();

    if (!digitalRead(BUTTON_PIN)) {
        settingsManager.dropWifiCredits();
        ESP_LOGI("*", "WiFi credits were removed!");
    }
}

String connectToWifi() {
    String ssid = settingsManager.getSettingString(GROUP_WIFI, SSID_SETTING);
    String password = settingsManager.getSettingString(GROUP_WIFI, PASSWORD_SETTING);
    
    if (ssid.isEmpty()) {
        ESP_LOGI("*", "Ssid is blank -> creating AP");
        WiFi.softAP("LOUVER");
        // WiFi.beginSmartConfig();
        delay(500);

        if (MDNS.begin("louver")) {
            MDNS.addService("http", "tcp", 80);
        } else {
            ESP_LOGI("*", "Failed to setup up MDNS");
        }

        return WiFi.softAPIP().toString();
    } else {
        ESP_LOGI("*", "WiFi connecting to %s :: %s", ssid.c_str(), password.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        long startTime = millis();
        ledIndicator.blink();
        while (!WiFi.isConnected() && millis() - startTime < WIFI_SETUP_TIMEOUT) {}
        ledIndicator.off();
        if (WiFi.isConnected()) {
            return WiFi.localIP().toString();
        } else {
            WiFi.disconnect();
            return "";
        }
    }
}

void setupServerEndPoints() {
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", buildMainPage(WiFi.getMode() == WIFI_MODE_AP));
    });
    
    if (WiFi.getMode() == WIFI_MODE_AP) {
        server.on("/setup", HTTP_POST, []() {
            ESP_LOGI(WEB_SERVER_TAG, "[POST] [/setup]");
            handleSetup(&server, &settingsManager);
        });
    }

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
        server.send(200, "application/json", settingsManager.getJson(GROUP_CONFIG));
    });
    server.on("/settings", HTTP_POST, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[POST] [/settings]");
        handleSettingsPost(&server, &settingsManager);
        processConfig();
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
    server.on("/restart", HTTP_PUT, [](){
        ESP_LOGI(WEB_SERVER_TAG, "[PUT] [/restart]");

        settingsManager.putSetting(GROUP_STATE, AUTOMODE_SETTING, controller.isAutoModeEnabled());

        settingsManager.saveSettings();
        server.send(200);

        delay(2000);
        ESP.restart();
    });
    server.onNotFound([](){
        server.send(404, "application/json", buildErrorJson("Page not found"));
    });
}

void processConfig() {
    JsonObject config = settingsManager.getSettings(GROUP_CONFIG);

    int lightClose = config[CLOSE_SETTING];
    int lightOpen = config[OPEN_SETTING];
    int lightBright = config[BRIGHT_SETTING];

    if (lightClose != 0 && lightOpen != 0 && lightBright != 0) {
        controller.setLightValues(lightClose, lightOpen, lightBright);
    }

    int delay = config[DELAY_SETTING];
    if (delay > 0) {
        controller.setMonitorTaskDelay(delay);
    }

    controller.restartAutoMode();
}