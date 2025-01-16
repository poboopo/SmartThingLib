// Enable and disable some features.
// You can edit them there or add build_flags in platformio.ini (-DENABLE_WEB_PAGE="0" for example).
// In some cases disabling unused feature can significantly decrease final firmware size.

#define TCP_LOGGER 1
#define MULTICAST_LOGGER 2
#define SERIAL_LOGGER 3

#define LOGGING_LEVEL_DEBUG 10
#define LOGGING_LEVEL_INFO 20
#define LOGGING_LEVEL_WARN 30
#define LOGGING_LEVEL_ERROR 40

#ifndef ENABLE_WEB_PAGE
  // esp8266 works kinda bad with full web ui
  #ifdef ARDUINO_ARCH_ESP8266
    #define ENABLE_WEB_PAGE 0
  #else
    #define ENABLE_WEB_PAGE 1
  #endif
#endif

// Enable actions
#ifndef ENABLE_ACTIONS
  #define ENABLE_ACTIONS 1
#endif

// Enable number sensors
#ifndef ENABLE_NUMBER_SENSORS
  #define ENABLE_NUMBER_SENSORS 1
#endif

// Enable text sensors
#ifndef ENABLE_TEXT_SENSORS
  #define ENABLE_TEXT_SENSORS 1
#endif

#if ENABLE_NUMBER_SENSORS || ENABLE_TEXT_SENSORS
  // Enable hooks for sensors
  #ifndef ENABLE_HOOKS
    #define ENABLE_HOOKS 1
  #endif
#else
  #define ENABLE_HOOKS 0
#endif

#if ENABLE_ACTIONS
  // Enable action call scheduler
  #ifndef ENABLE_ACTIONS_SCHEDULER
    #define ENABLE_ACTIONS_SCHEDULER 1
  #endif
#else
  #define ENABLE_ACTIONS_SCHEDULER 0
#endif

// Enable configuration
#ifndef ENABLE_CONFIG
  #define ENABLE_CONFIG 1
#endif

// Enable ArduinoOTA
#ifndef ENABLE_OTA
  #define ENABLE_OTA 1
#endif

// Enable logger
#ifndef ENABLE_LOGGER
  #define ENABLE_LOGGER 1
#endif

#if ENABLE_CONFIG
  #ifndef LOGGER_TYPE
    #define LOGGER_TYPE TCP_LOGGER
  #endif
#else
  #define LOGGER_TYPE SERIAL_LOGGER
#endif

#ifndef LOGGING_LEVEL
  #define LOGGING_LEVEL LOGGING_LEVEL_INFO
#endif
