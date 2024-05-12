#include "logs/BetterLogger.h"

#if LOGGER_TYPE == SERIAL_LOGGER

BetterLogger LOGGER;

void BetterLogger::connect() {}

void BetterLogger::log(const char* message) {
  Serial.println(message);
}

#endif