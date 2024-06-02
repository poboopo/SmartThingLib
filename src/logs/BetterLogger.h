#ifndef BetterLogger_H
#define BetterLogger_H

#include "Features.h"
#include <Arduino.h>
#include <lwip/sockets.h>

#define LOGGER_TAG "LOGGER"

#if LOGGER_TYPE == SERIAL_LOGGER
// [name][level][tag]message
#define LOGGER_MESSAGE_TEMPLATE "[%s][%u][%s]%s"
#else
// name_&_level_&_tag_&_message
#define LOGGER_MESSAGE_TEMPLATE "%s_&_%u_&_%s_&_%s\n"
#endif

#define STAT_LOG_TAG "STATISTICS"
#define MAX_MESSAGE_LENGTH 2048

class BetterLogger {
 public:
  BetterLogger(){};
  ~BetterLogger() { 
    #if ENABLE_LOGGER
    close(_sock);
    #endif
  }

  void init() { Serial.begin(115200); }
  void initConnection(String fullAddr, const char* name) {
    #if ENABLE_LOGGER
    _name = name;
    _fullAddr = fullAddr;

    connectToServer();
    #endif
  }
  bool isConnected() {
    return _connected;
  }
  void configUpdateHook(String fullAddr) {
    #if ENABLE_LOGGER && LOGGER_TYPE != SERIAL_LOGGER
    if (_connected && _fullAddr.equals(fullAddr)) {
      return;
    }
    _fullAddr = fullAddr;
    warning(LOGGER_TAG, "Server log address was updated");
    connectToServer();
    #endif
  }

  #if ENABLE_LOGGER
  void log(const char* message);
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
  #if ENABLE_LOGGER
  bool _connected = false;
  int _sock;
  struct sockaddr_in _saddr = {0};
  String _fullAddr;

  const char* _name = "no_name";

  void connectToServer();

  bool parseAddressFromString() {
    if (_fullAddr.isEmpty()) {
      warning(LOGGER_TAG, "Empty tcp log server info");
      return false;
    }

    int ind = _fullAddr.indexOf(":");
    if (ind < 0) {
      error(LOGGER_TAG, "Bad server fullAddr: %s", _fullAddr.c_str());
      return false;
    }

    String ip = _fullAddr.substring(0, ind);
    String port = _fullAddr.substring(ind + 1);
    if (ip.isEmpty()) {
      error(LOGGER_TAG, "Failed to parse server ip");
    }
    if (port.isEmpty()) {
      error(LOGGER_TAG, "Failed to parse server port");
    }
    _saddr.sin_family = AF_INET;
    _saddr.sin_port = htons(port.toInt());
    _saddr.sin_addr.s_addr = inet_addr(ip.c_str());
    return true;
  };
  #endif
};

extern BetterLogger LOGGER;

#endif