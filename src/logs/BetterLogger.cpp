#include "logs/BetterLogger.h"

// ip$name$level$tag$message
#define LOGGER_MESSAGE_TEMPLATE "%s$%s$%s$%s$%s"
#define STAT_LOG_TAG "STATISTICS"

BetterLogger LOGGER;

BetterLogger::BetterLogger() {
}

BetterLogger::~BetterLogger() {
    _multicaster.stop();
}

void BetterLogger::init() {
    Serial.begin(115200);
}

void BetterLogger::connect(const char * myIp, const char * name, const char * group, int port) {
    _multicaster.init(group, port);
    _name = name;
    _ip = myIp;
    _connected = true;
}

void BetterLogger::log(const char * level, const char * tag, const char * message) {
    char formattedMessage[MAX_MESSAGE_LENGTH];
    sprintf(formattedMessage,
        LOGGER_MESSAGE_TEMPLATE,
        _ip, 
        _name,
        level,
        tag, 
        message
    );

    xSemaphoreTake(_mutex, portMAX_DELAY);
    Serial.println(formattedMessage);
    if (_connected) {
        _multicaster.broadcast(formattedMessage);
    }
    xSemaphoreGive(_mutex);
}

void BetterLogger::statistics() {
#ifdef ENABLE_STATISTICS
    info(STAT_LOG_TAG, "----------STATISTIC----------");
    info(STAT_LOG_TAG, "Free/size heap: %u/%u", ESP.getFreeHeap(), ESP.getHeapSize());
    info(STAT_LOG_TAG, "Min free/max alloc heap: %u/%u", ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
    info(STAT_LOG_TAG, "--------STATISTIC-END--------");
#endif
}