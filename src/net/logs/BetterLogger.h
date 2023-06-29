#ifndef BetterLogger_H
#define BetterLogger_H

#include <Arduino.h>
#include <freertos/semphr.h>

#include "net/socket/Multicaster.h"

#define ENABLE_STATISTICS

#define LOGGER_DEFAULT_GROUP "224.1.1.1"
#define LOGGER_DEFAULT_PORT 7779
#define MAX_MESSAGE_LENGTH 2048

// move to config?
// #define LOGGING_LEVEL_ERROR
// #define LOGGING_LEVEL_WARN
// #define LOGGING_LEVEL_INFO
// #define LOGGING_LEVEL_DEBUG
#define LOGGING_LEVEL_ALL


class BetterLogger {
    public:
        BetterLogger();
        ~BetterLogger();

        void init();

        void connect(const char * myIp, const char * name){
            connect(myIp, name, LOGGER_DEFAULT_GROUP, LOGGER_DEFAULT_PORT);
        };
        void connect(const char * myIp, const char * name, const char * group, int port);

        void log(const char * level, const char * tag, const char * message);
        template<typename... Args>
        void log(const char * level, const char * tag, const char * format, Args... args) {
            // TODO fix long args will cause core panic
            char message[MAX_MESSAGE_LENGTH];
            sprintf(message, format, args...);
            log(level, tag, message);
        }
        
        void logRequest(const char * tag, const char *  method, const char *  uri, const char *  body) {
            info(tag, "[%s] %s - %s", method, uri, body);
        };

        //bad realization, but i have no other ideas
        template<typename... Args>
        void error(const char * tag, const char * format, Args... args) {
            #if defined(LOGGING_LEVEL_ERROR) || defined(LOGGING_LEVEL_WARN) || defined(LOGGING_LEVEL_INFO) || defined(LOGGING_LEVEL_DEBUG) || defined(LOGGING_LEVEL_ALL)
            log("ERROR", tag, format, args...);
            #endif
        };
        template<typename... Args>
        void warning(const char * tag, const char * format, Args... args) {
            #if defined(LOGGING_LEVEL_WARN) || defined(LOGGING_LEVEL_INFO) || defined(LOGGING_LEVEL_DEBUG) || defined(LOGGING_LEVEL_ALL)
            log("WARNING", tag, format, args...);
            #endif
        };
        template<typename... Args>
        void info(const char * tag, const char * format, Args... args) {
            #if defined(LOGGING_LEVEL_INFO) || defined(LOGGING_LEVEL_DEBUG) || defined(LOGGING_LEVEL_ALL)
            log("INFO", tag, format, args...);
            #endif
        };
        template<typename... Args>
        void debug(const char * tag, const char * format, Args... args) {
            #if defined(LOGGING_LEVEL_DEBUG) || defined(LOGGING_LEVEL_ALL)
            log("DEBUG", tag, format, args...);
            #endif
        };
        void statistics(); 
    private:
        Multicaster _multicaster;
        SemaphoreHandle_t _mutex = xSemaphoreCreateMutex();
        const char * _ip = "NOT_CONNECTED";
        const char * _name = "no_name";
        bool _connected = false;
        bool _serialStarted = false;
};

extern BetterLogger LOGGER;

#endif