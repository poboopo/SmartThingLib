#include "logs/BetterLogger.h"

// todo esp8266
#if defined(ARDUINO_ARCH_ESP32) && ENABLE_LOGGER && LOGGER_TYPE == MULTICAST_LOGGER

BetterLogger LOGGER;

bool BetterLogger::connect(const char * ip, int port) {
  IPAddress address;
  address.fromString(ip);
  return _udp.beginMulticast(address, port);
}

void BetterLogger::disconnect() {
  _udp.stop();
}

size_t BetterLogger::sendRemote(const char* message) {
  _udp.beginMulticastPacket();
  size_t sent = _udp.write((uint8_t *) message, strlen(message));
  _udp.endPacket();
  return sent;
}
#endif