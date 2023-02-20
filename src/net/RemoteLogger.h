#include <net/Multicaster.h>

#include <Arduino.h>

#define LOGGER_DEFAULT_GROUP "224.1.1.1"
#define LOGGER_DEFAULT_PORT 7779

#define MAX_MESSAGE_LENGTH 100

struct LoggerMessage {
    long time;
    char message[MAX_MESSAGE_LENGTH];
    LoggerMessage * next = NULL;
};

class RemoteLogger {
    public:
        RemoteLogger();
        ~RemoteLogger();

        void init();
        void init(const char * group, int port);
        void log(const char * message);
        void test();
    private:
        Multicaster _multicaster;
        const char * _ip;
        LoggerMessage * _firstMessage = NULL;
        LoggerMessage * _lastMessage = NULL;

        void addToQueue(char * message);
        LoggerMessage * popFromQueue();
};