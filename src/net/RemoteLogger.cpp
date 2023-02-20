#include <net/RemoteLogger.h>

RemoteLogger::RemoteLogger(){}
RemoteLogger::~RemoteLogger() {
    _multicaster.stop();
}

void RemoteLogger::init() {
    init(LOGGER_DEFAULT_GROUP, LOGGER_DEFAULT_PORT);
}

void RemoteLogger::init(const char * group, int port) {
    _multicaster.init(group, port);   
}

void RemoteLogger::log(const char * message) {
    _multicaster.broadcast(message);
}

void RemoteLogger::test() {
    // for (int i = 0; i < 4; i++) {
    //     char * buff;
    //     sprintf(buff, "message %d", i);
    //     addToQueue(buff);
    // }

    // LoggerMessage * message;
    // while((message = popFromQueue()) != NULL) {
    //     ESP_LOGI("1", "%s", message->message);
    // }

    for (int i = 0; i < 4; i++) {
        char buff[20];
        sprintf(buff, "message %d", i);
        _multicaster.broadcast(buff);
        delay(500);
    }
}

void RemoteLogger::addToQueue(char * message) {
    LoggerMessage * newMessage = (struct LoggerMessage * ) malloc(sizeof(LoggerMessage));
    newMessage->time = millis();
    strncpy(newMessage->message, message, MAX_MESSAGE_LENGTH);

    if (_firstMessage == NULL) {
        memcpy(_firstMessage, newMessage, sizeof(LoggerMessage));
    } else if (_firstMessage != NULL && _lastMessage == NULL) {
        memcpy(_firstMessage->next, newMessage, sizeof(LoggerMessage));
        memcpy(_lastMessage, newMessage, sizeof(LoggerMessage));
    } else {
        memcpy(_lastMessage->next, newMessage, sizeof(LoggerMessage));
        memcpy(_lastMessage, newMessage, sizeof(LoggerMessage));
    }
}

LoggerMessage * RemoteLogger::popFromQueue() {
    if (_firstMessage == NULL) {
        return NULL;
    }

    LoggerMessage * oldFirst = _firstMessage;
    free(_firstMessage);
    if (oldFirst->next != NULL) {
        memcpy(_firstMessage, oldFirst->next, sizeof(LoggerMessage));
    }
    return oldFirst;
}