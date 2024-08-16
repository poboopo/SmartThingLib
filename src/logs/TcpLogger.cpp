#include "logs/BetterLogger.h"
#include "settings/SettingsManager.h"

#if ENABLE_LOGGER && LOGGER_TYPE == TCP_LOGGER

BetterLogger LOGGER;

bool BetterLogger::connect(const char * ip, int port) {
  IPAddress address;
  address.fromString(ip);
  return _tcp.connect(address, port);
}

void BetterLogger::disconnect() {
  _tcp.stop();
}

size_t BetterLogger::sendRemote(const char* message) {
  return _tcp.write((uint8_t *) message, strlen(message));
}

#endif