// Enable and disable features
// Use build_flags in platformio.ini to acces this flags

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

#ifndef ENABLE_HOOKS
#define ENABLE_HOOKS 1
#endif

#ifndef ENABLE_LOGGER
#define ENABLE_LOGGER 1
#endif

#ifndef LOGGER_TYPE
// #define LOGGER_TYPE TCP_LOGGER
#define LOGGER_TYPE SERIAL_LOGGER
#endif

#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL LOGGING_LEVEL_DEBUG
#endif
