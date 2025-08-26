## Feature Flags for Disabling Library Functionality

Example usage for **PlatformIO**:

```
build_flags = 
    '-D__VERSION="1.0"'
    '-DLOGGING_LEVEL=10'
```

### Available Flags:

* **ENABLE_WEB_PAGE** – enable the full version of the web page;
* **ENABLE_ACTIONS** – enable actions functionality;
* **ENABLE_ACTIONS_SCHEDULER** – enable the functionality for periodically calling actions. Disabled if `ENABLE_ACTIONS == 0`;
* **ENABLE_NUMBER_SENSORS** – enable numeric sensors functionality;
* **ENABLE_TEXT_SENSORS** – enable text sensors functionality;
* **ENABLE_HOOKS** – enable hooks functionality. Automatically disabled if `ENABLE_NUMBER_SENSORS == 0 && ENABLE_TEXT_SENSORS == 0`;
* **ENABLE_CONFIG** – enable device configuration functionality;
* **ENABLE_OTA** – enable ArduinoOTA;
* **ENABLE_LOGGER** – enable logging;
* **LOGGER_TYPE** – choose logger implementation. Two main options:

  * `1` – network TCP logger (default);
  * `3` – serial logger;
* **LOGGING_LEVEL** – logging level:

  * `DEBUG` – 10;
  * `INFO` – 20;
  * `WARN` – 30;
  * `ERROR` – 40.

Additionally, in the build parameters, the developer can specify the firmware version using the `__VERSION` parameter.