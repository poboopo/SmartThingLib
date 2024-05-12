#include "logs/BetterLogger.h"

#if ENABLE_LOGGER && LOGGER_TYPE == MULTICAST_LOGGER

BetterLogger LOGGER;

void BetterLogger::connect() {
  if (!parseAddressFromString()) {
    return;
  }
  if (_sock >= 0) {
    close(_sock);
    _connected = false;
  }
  info(LOGGER_TAG, "Connecting to %s via udp multicast", _fullAddr.c_str());

  _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (_sock < 0) {
    close(_sock);
    Serial.println("Socket creation failed");
    return;
  }

  _connected = true;
  log("DEBUG", "LOGGER", "Connected to multicast group");
}

void BetterLogger::log(const char* message) {
  Serial.println(message);

  // is it really necessary?
  if (_connected && _sock >= 0) {
    int nbytes = sendto(_sock, message, strlen(message), 0,
                        (struct sockaddr*)&_saddr, sizeof(_saddr));
    if (nbytes < 0) {
      Serial.println("Failed to send message via socket. Closing connection.");
      close(_sock);
      _sock = -1;
      _connected = false;
    }
  }
}
#endif