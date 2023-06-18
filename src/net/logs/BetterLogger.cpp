#include "net/logs/BetterLogger.h"

#define LOGGER_MESSAGE_TEMPLATE "{%s}[%s][%s]::%s"

Multicaster BetterLogger::_multicaster = Multicaster();
bool BetterLogger::_connected = false;
bool BetterLogger::_serialStarted = false;
SemaphoreHandle_t BetterLogger::_mutex = xSemaphoreCreateMutex();
const char * BetterLogger::_ip = "NOT_CONNECTED";
const char * BetterLogger::_name = "no_name";

BetterLogger::BetterLogger() {
}

BetterLogger::~BetterLogger() {
    BetterLogger::_multicaster.stop();
}

void BetterLogger::init() {
    Serial.begin(115200);
}

void BetterLogger::connect(const char * myIp, const char * name, const char * group, int port) {
    BetterLogger::_multicaster.init(group, port);
    BetterLogger::_name = name;
    BetterLogger::_ip = myIp;
    BetterLogger::_connected = true;
}

void BetterLogger::log(const char * tag, const char * message) {
    char formattedMessage[MAX_MESSAGE_LENGTH];

    sprintf(formattedMessage, LOGGER_MESSAGE_TEMPLATE, _ip, _name, tag, message);

    xSemaphoreTake(_mutex, portMAX_DELAY);
    Serial.println(formattedMessage);
    if (_connected) {
        _multicaster.broadcast(formattedMessage);
    }
    xSemaphoreGive(_mutex);
}

void BetterLogger::statistics() {
#ifdef ENABLE_STATISTICS
    log(STAT_LOG_TAG, "----------STATISTIC----------");
    log(STAT_LOG_TAG, "Free/size heap: %u/%u", ESP.getFreeHeap(), ESP.getHeapSize());
    log(STAT_LOG_TAG, "Min free/max alloc heap: %u/%u", ESP.getMinFreeHeap(), ESP.getMaxAllocHeap());
    log(STAT_LOG_TAG, "--------STATISTIC-END--------");
#endif
}