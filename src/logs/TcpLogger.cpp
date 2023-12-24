#include "logs/BetterLogger.h"
#include "settings/SettingsManager.h"

#ifdef TCP_LOGGER

BetterLogger LOGGER;

void BetterLogger::connectSocket() {
  if (!parseAddressFromString()) {
    return;
  }
  if (_sock >= 0) {
    close(_sock);
    _connected = false;
  }

  info(LOGGER_TAG, "Connecting to %s via tcp", _fullAddr.c_str());

  _sock = socket(AF_INET, SOCK_STREAM, 0);
  if (_sock < 0) {
    Serial.println("Socket creation failed");
    return;
  }

  // move to async task
  if (connect(_sock, (struct sockaddr*)&_saddr, sizeof(_saddr)) < 0) {
    error(LOGGER_TAG, "Connection failed");
    return;
  }
  _connected = true;
  info(LOGGER_TAG, "Connected to tcp logging server");
}

void BetterLogger::log(const char* level, const char* tag,
                       const char* message) {
  char formattedMessage[MAX_MESSAGE_LENGTH];
  sprintf(formattedMessage, LOGGER_MESSAGE_TEMPLATE, _ip, _name, level, tag,
          message);
  Serial.println(formattedMessage);

  // is it really necessary?
  if (_connected && _sock > 0) {
    int nbytes = send(_sock, formattedMessage, strlen(formattedMessage), 0);
    if (nbytes < 0) {
      Serial.println("Failed to send message via socket. Closing connection.");
      close(_sock);
      _sock = -1;
      _connected = false;
    }
  }
}

#endif