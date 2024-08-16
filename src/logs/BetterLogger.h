#ifndef BetterLogger_H
#define BetterLogger_H

#include "Features.h"
#include <Arduino.h>

#define LOGGER_TAG "LOGGER"

#if LOGGER_TYPE == SERIAL_LOGGER
// [name][level][tag]message
#define LOGGER_MESSAGE_TEMPLATE "[%s][%u][%s]%s"
#else
// name_&_level_&_tag_&_message
#define LOGGER_MESSAGE_TEMPLATE "%s_&_%u_&_%s_&_%s\n"
#endif

#if LOGGER_TYPE == MULTICAST_LOGGER
#include <WiFiUdp.h>
#endif
#if LOGGER_TYPE == TCP_LOGGER
#include <WiFiClient.h>
#endif

#define STAT_LOG_TAG "STATISTICS"
#define MAX_MESSAGE_LENGTH 2048

class BetterLogger {
 public:
  BetterLogger(){};
  ~BetterLogger() { 
    #if LOGGER_TYPE != SERIAL_LOGGER
    disconnect();
    #endif
  }

  void init() { Serial.begin(115200); }
  void init(String fullAddr, const char* name) {
    _name = name;
    #if LOGGER_TYPE != SERIAL_LOGGER
    _fullAddr = fullAddr;

    parseAddressAndConnect();
    #endif
  }
  bool isConnected() {
    #if LOGGER_TYPE == SERIAL_LOGGER
    return true;
    #else
    return _connected;
    #endif
  }
  void updateAddress(String fullAddr) {
    #if ENABLE_LOGGER && LOGGER_TYPE != SERIAL_LOGGER
    if (_connected && _fullAddr.equals(fullAddr)) {
      return;
    }
    _fullAddr = fullAddr;
    warning(LOGGER_TAG, "Server log address was updated");
    parseAddressAndConnect();
    #endif
  }

  #if ENABLE_LOGGER
  void log(const char* message) {
    Serial.println(message);
    #if LOGGER_TYPE != SERIAL_LOGGER
    if (_connected) {
      if (sendRemote(message) < 0) {
        _connected = false;
        error(LOGGER_TAG, "Failed to send message to remote, closing connection");
      }
    }
    #endif
  }

  template <typename... Args>
  void log(uint8_t level, const char* tag, const char* format,
           Args... args) {
    // TODO fix long args will cause core panic
    char message[MAX_MESSAGE_LENGTH];
    sprintf(message, format, args...);
    char formattedMessage[MAX_MESSAGE_LENGTH];
    sprintf(formattedMessage, LOGGER_MESSAGE_TEMPLATE, _name, level, tag,
            message);
    log(formattedMessage);
  }
  #else
    void log(const char* message){}
    template <typename... Args>
    void log(uint8_t level, const char* tag, const char* format, Args... args) {
    }
  #endif

  void logRequest(const char* tag, const char* method, const char* uri,
                  const char* body) {
    info(tag, "[%s] %s - %s", method, uri, body == nullptr ? "" : body);
  };

  // bad impl, but i have no other ideas
  template <typename... Args>
  void error(const char* tag, const char* format, Args... args) {
#if LOGGING_LEVEL == LOGGING_LEVEL_ERROR || LOGGING_LEVEL == LOGGING_LEVEL_WARN || LOGGING_LEVEL == LOGGING_LEVEL_INFO || LOGGING_LEVEL == LOGGING_LEVEL_DEBUG
    log(LOGGING_LEVEL_ERROR, tag, format, args...);
#endif
  };
  template <typename... Args>
  void warning(const char* tag, const char* format, Args... args) {
#if LOGGING_LEVEL == LOGGING_LEVEL_WARN || LOGGING_LEVEL == LOGGING_LEVEL_INFO || LOGGING_LEVEL == LOGGING_LEVEL_DEBUG
    log(LOGGING_LEVEL_WARN, tag, format, args...);
#endif
  };
  template <typename... Args>
  void info(const char* tag, const char* format, Args... args) {
#if LOGGING_LEVEL == LOGGING_LEVEL_INFO || LOGGING_LEVEL == LOGGING_LEVEL_DEBUG
    log(LOGGING_LEVEL_INFO, tag, format, args...);
#endif
  };
  template <typename... Args>
  void debug(const char* tag, const char* format, Args... args) {
#if LOGGING_LEVEL == LOGGING_LEVEL_DEBUG
    log(LOGGING_LEVEL_DEBUG, tag, format, args...);
#endif
  };
  void statistics() {
    info(STAT_LOG_TAG, "----------STATISTIC----------");
    info(STAT_LOG_TAG, "Free/size heap: %u/%u", ESP.getFreeHeap(),
         ESP.getHeapSize());
    info(STAT_LOG_TAG, "Min free/max alloc heap: %u/%u", ESP.getMinFreeHeap(),
         ESP.getMaxAllocHeap());
    info(STAT_LOG_TAG, "--------STATISTIC-END--------");
  };

 private:
  const char* _name = "no_name";
  #if LOGGER_TYPE != SERIAL_LOGGER
  bool _connected = false;
  String _fullAddr;

  #if LOGGER_TYPE == MULTICAST_LOGGER
  WiFiUDP _udp;
  #endif
  #if LOGGER_TYPE == TCP_LOGGER
  WiFiClient _tcp;
  #endif

  size_t sendRemote(const char * message);
  bool connect(const char * ip, int port);
  void disconnect();

  void parseAddressAndConnect() {
    if (_fullAddr.isEmpty()) {
      warning(LOGGER_TAG, "Empty tcp log server info");
      return;
    }

    int ind = _fullAddr.indexOf(":");
    if (ind < 0) {
      error(LOGGER_TAG, "Bad server fullAddr: %s, need ip:port", _fullAddr.c_str());
      return;
    }

    String ip = _fullAddr.substring(0, ind);
    String port = _fullAddr.substring(ind + 1);
    if (ip.isEmpty()) {
      error(LOGGER_TAG, "Failed to parse server ip");
      return;
    }
    if (port.isEmpty()) {
      error(LOGGER_TAG, "Failed to parse server port");
      return;
    }

    info(LOGGER_TAG, "Trying to connect to logger server [%s, %s]", ip.c_str(), port.c_str());
    disconnect();
    _connected = false;
    if (connect(ip.c_str(), port.toInt())) {
      _connected = true;
      info(LOGGER_TAG, "Logger connected!");
    } else {
      error(LOGGER_TAG, "Failed to connect");
    }
  };
  #endif
};

extern BetterLogger LOGGER;

#endif