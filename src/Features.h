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
#define ENABLE_WEB_PAGE 1
#endif

#ifndef ENABLE_ACTIONS
#define ENABLE_ACTIONS 1
#endif

#ifndef ENABLE_SENSORS
#define ENABLE_SENSORS 1
#endif

#ifndef ENABLE_STATES
#define ENABLE_STATES 1
#endif

#if ENABLE_SENSORS && ENABLE_STATES
  #ifndef ENABLE_HOOKS
    #define ENABLE_HOOKS 1
  #endif
#else
  #define ENABLE_HOOKS 0
#endif

#if ENABLE_ACTIONS
  #ifndef ENABLE_ACTIONS_SCHEDULER
    #define ENABLE_ACTIONS_SCHEDULER 1
  #endif
#else
  #define ENABLE_ACTIONS_SCHEDULER 0
#endif

#ifndef ENABLE_LOGGER
#define ENABLE_LOGGER 1
#endif

#ifndef LOGGER_TYPE
#define LOGGER_TYPE TCP_LOGGER
#endif

#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL LOGGING_LEVEL_INFO
#endif
