# SmartThingLib
The `SmartThingLib` library is designed to significantly accelerate IoT device development by implementing most of the basic functionality. Supported platforms: `esp32` and `esp8266`.
To create a device, you only need to specify its type, what actions it can perform, what sensors it includes, and the possible configuration settings.
The finished device comes with a web interface for control and configuration.

A gateway application and cloud platform for managing multiple devices are also implemented. You can find them on the [SmartThing main project page](https://github.com/poboopo/SmartThingProject).

## Device Web Interface

|                                      Information                                     |                                     WiFi Settings                                    |
| :----------------------------------------------------------------------------------: | :----------------------------------------------------------------------------------: |
| ![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/info.png?raw=true) | ![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/wifi.png?raw=true) |

|                                         Actions                                         |                                    Sensors and Hooks                                    |
| :-------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------: |
| ![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/actions.png?raw=true) | ![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/sensors.png?raw=true) |

|                                         Settings                                         |                                         Metrics                                         |
| :--------------------------------------------------------------------------------------: | :-------------------------------------------------------------------------------------: |
| ![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/settings.png?raw=true) | ![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/metrics.png?raw=true) |

## How to Use

The following libraries are required:
```
bblanchon/ArduinoJson@^7.0.4
esphome/ESPAsyncWebServer-esphome@^3.2.2
```

During device initialization, you need to set up all configurations.

### Adding a Device Action:

```C++
/*
    When adding an action, you need to specify:
    - its system name
    - the text for displaying in the UI
    - a lambda with the action logic that returns true or false
*/
ActionsManager.add("led_on", "Turn led on", []() {
    digitalWrite(LED_PIN, LOW);
    return true;
});
```

### Supported Sensor Types:

Example of adding standard sensors to read from digital and analog pins:

```C++
// Adds a sensor with the specified name, configures the pin, and reads its value using digitalRead
SensorsManager.addDigital("button", 12);
// Reads the value from a pin using analogRead
SensorsManager.addAnalog("analog", 14);
```

If sensor readings come from other logic, you can add them by specifying a function for value calculation. Both numeric (`long`) and text (`String`) values are supported:

```C++
// Numeric sensor (default type: long)
// Adding a numeric sensor with custom logic
SensorsManager.add("sensor", []() {
    return tempSensor.readData();
});
// Text sensor (default type: String)
// Adding a text sensor
SensorsManager.add("led", []() {
    // Example logic for computing the value
    return digitalRead(LED_PIN) == HIGH ? "on" : "off";
});
```

### Adding Configuration Options:

```C++
// Add configuration by specifying a unique key name
ConfigManager.add("test-value");

// Get its value in code by key
ConfigManager.get("test-value");

// Add a listener for when the device configuration updates
ConfigManager.onConfigUpdate([]() {
    st_log_debug("main", "Config updated!");
});
```

Configuration values are stored as strings in the device `EEPROM` and persist after reboot.

### Initialize `SmartThing`:

```C++
// Specify the device type
if (!SmartThing.init("lamp")) {
    st_log_error("main", "Failed to init SmartThing!");
    while(true) {
        delay(1000);
    }
}
```

If using `esp8266`, add a call to `SmartThing.loop()` in the main loop:

```C++
void loop() {
    SmartThing.loop();
    delay(250);
}
```

For `esp32`, this is not needed, as the loop method runs in an asynchronous task.

A full example of the firmware can be found in the [test repository](https://github.com/poboopo/SmartThingLibTest/blob/master/src/main.cpp).

## First Launch

On first boot, the device will create an access point named `st-device`. Connect to it and navigate to [`http://192.168.1.4/`](http://192.168.1.4/) in your browser — the device control panel will open, where you can configure WiFi. After setup, restart the device.


## Additional Features

### Hooks and Triggers

For sensors, users can add **hooks** — logic that executes when a trigger occurs. A trigger is a change in sensor value.
You can specify an exact trigger value, a range, or a threshold for numeric sensors.

Currently, three hook types are implemented:

* **action** — executes a specified device action;
* **http** — sends an HTTP request based on parameters;
* **notification** — sends a notification to the `SmartThingGateway` (gateway). The gateway address comes from device configuration.

HTTP and notification hooks support placeholders in text fields for:

* current sensor value (`{v}`)
* configuration values (`{key}`).

Example: if you need to send a request with a sensor value to a server, add a configuration key `server_address` and reference it as `{server_address}`. Then add an `http` hook like this:

![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/hook.png?raw=true)

If `button = 1` and `server_address = "192.168.1.2"` when the hook runs, then `payload` becomes `{"value": "1"}` and `url` becomes `http://192.168.1.2/api`.

To test a hook, use the test button:

![](https://github.com/poboopo/SmartThingLib/blob/docs/doc/assets/hook_test.png?raw=true)


### Logging

The library has its own network logger with macros:
`st_log_error`, `st_log_warning`, `st_log_info`, `st_log_debug`.
By default, logs go to the serial output, but you can specify a TCP server (`laddr`) in device settings. The format is `ip:port`. The gateway application includes a built-in log collection server.


### Feature Flags

Unused functionality can be disabled with feature flags during build, reducing firmware size.
Read more in [FEATURE\_FLAGS.md](https://github.com/poboopo/SmartThingLib/blob/master/FEATURE_FLAGS.md).


## Resetting Settings

To reset settings, go to the **Danger zone** tab and click the factory reset button.
If the web interface is not accessible, short pin `19` (esp32) or `D4` (esp8266) to ground for 5 seconds — all settings will be fully reset.


## Examples

* [Test project](https://github.com/poboopo/SmartThingLibTest/blob/master/src/main.cpp)
* [Weather station](https://github.com/PavelProjects/meteo_station)
* [Automatic blinds](https://github.com/PavelProjects/SmarThingtLouver)
