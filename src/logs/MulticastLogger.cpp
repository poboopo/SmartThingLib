#include "logs/BetterLogger.h"

#ifdef MULTICAST_LOGGER

#define LOGGER_DEFAULT_GROUP "224.1.1.1"
#define LOGGER_DEFAULT_PORT 7779

BetterLogger LOGGER;

void BetterLogger::configUpdateHook() {}

void BetterLogger::connectSocket() {
    if (!parseAddressFromString()) {
        return;
    }
    if (_sock >= 0) {
        close(_sock);
        _connected = false;
    }
    info(LOGGER_TAG, "Connecting to %s via udp multicast", _fullAddr.c_str());

    _sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_sock < 0){
        close(_sock);
        Serial.println("Socket creation failed");
        return;
    }

    _connected = true;
    log("DEBUG", "LOGGER", "Connected to multicast group");
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
    Serial.println(formattedMessage);

    // is it really necessary?
    if (_connected && _sock >= 0) {
        int nbytes = sendto(
            _sock,
            formattedMessage,
            strlen(formattedMessage),
            0,
            (struct sockaddr*) &_saddr,
            sizeof(_saddr)
        );
        if (nbytes < 0) {
            Serial.println("Failed to send message via socket. Closing connection.");
            close(_sock);
            _sock = -1;
            _connected = false;
        }
    }
}
#endif