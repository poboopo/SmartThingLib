#ifndef BetterLogger_H
#define BetterLogger_H

#include <Arduino.h>
#include <freertos/semphr.h>

#include "net/socket/Multicaster.h"

#define ENABLE_STATISTICS

#define LOGGER_DEFAULT_GROUP "224.1.1.1"
#define LOGGER_DEFAULT_PORT 7779

#define MAX_MESSAGE_LENGTH 2048
#define DEFAULT_LOG_TAG "*"
#define STAT_LOG_TAG "STATISTICS"

#define STAT_LOG_DELAY 6000

class BetterLogger {
    public:
        ~BetterLogger();

        static void init();

        static void connect(const char * myIp, const char * name){
            connect(myIp, name, LOGGER_DEFAULT_GROUP, LOGGER_DEFAULT_PORT);
        };
        static void connect(const char * myIp, const char * name, const char * group, int port);

        static void logRequest(const char * tag, const char *  method, const char *  uri, const char *  body) {
            log(tag, "[%s] %s - %s", method, uri, body);
        };
        static void log(const char * tag, const char * message);
        static void log(const char * message) {
            log(DEFAULT_LOG_TAG, message);
        };
        template<typename... Args>
        static void log(const char * tag, const char * format, Args... args) {
            // TODO fix long args will cause core panic
            char message[MAX_MESSAGE_LENGTH];
            sprintf(message, format, args...);
            log(tag, message);
        };

        static void statistics(); 
    private:
        BetterLogger();

        static Multicaster _multicaster;
        static SemaphoreHandle_t _mutex;
        static const char * _ip;
        static const char * _name;
        static bool _connected;
        static bool _serialStarted;
};

#endif