#ifndef BetterLogger_H
#define BetterLogger_H

#include <Arduino.h>
#include <lwip/sockets.h>

#define LOGGER_TAG "LOGGER"

// #define MULTICAST_LOGGER
#define TCP_LOGGER

// name_&_level_&_tag_&_message
#define LOGGER_MESSAGE_TEMPLATE "%s_&_%s_&_%s_&_%s\n"
#define STAT_LOG_TAG "STATISTICS"
#define MAX_MESSAGE_LENGTH 2048

// move to config?
// #define LOGGING_LEVEL_ERROR
// #define LOGGING_LEVEL_WARN
// #define LOGGING_LEVEL_INFO
// #define LOGGING_LEVEL_DEBUG
#define LOGGING_LEVEL_ALL

// todo add websocker support?
class BetterLogger {
 public:
  BetterLogger(){};
  ~BetterLogger() { close(_sock); }

  void init() { Serial.begin(115200); }
  void initNetConnection(String fullAddr, const char* name) {
    _name = name;
    _fullAddr = fullAddr;

    connectSocket();
  }
  void configUpdateHook(String fullAddr) {
    if (_connected && _fullAddr.equals(fullAddr)) {
      return;
    }
    _fullAddr = fullAddr;
    warning(LOGGER_TAG, "Tcp server log address was updated");
    connectSocket();
  }

  void log(const char* level, const char* tag, const char* message);
  template <typename... Args>
  void log(const char* level, const char* tag, const char* format,
           Args... args) {
    // TODO fix long args will cause core panic
    char message[MAX_MESSAGE_LENGTH];
    sprintf(message, format, args...);
    log(level, tag, message);
  }

  void logRequest(const char* tag, const char* method, const char* uri,
                  const char* body) {
    info(tag, "[%s] %s - %s", method, uri, body == nullptr ? "" : body);
  };

  // bad impl, but i have no other ideas
  template <typename... Args>
  void error(const char* tag, const char* format, Args... args) {
#if defined(LOGGING_LEVEL_ERROR) || defined(LOGGING_LEVEL_WARN) || \
    defined(LOGGING_LEVEL_INFO) || defined(LOGGING_LEVEL_DEBUG) || \
    defined(LOGGING_LEVEL_ALL)
    log("ERROR", tag, format, args...);
#endif
  };
  template <typename... Args>
  void warning(const char* tag, const char* format, Args... args) {
#if defined(LOGGING_LEVEL_WARN) || defined(LOGGING_LEVEL_INFO) || \
    defined(LOGGING_LEVEL_DEBUG) || defined(LOGGING_LEVEL_ALL)
    log("WARN", tag, format, args...);
#endif
  };
  template <typename... Args>
  void info(const char* tag, const char* format, Args... args) {
#if defined(LOGGING_LEVEL_INFO) || defined(LOGGING_LEVEL_DEBUG) || \
    defined(LOGGING_LEVEL_ALL)
    log("INFO", tag, format, args...);
#endif
  };
  template <typename... Args>
  void debug(const char* tag, const char* format, Args... args) {
#if defined(LOGGING_LEVEL_DEBUG) || defined(LOGGING_LEVEL_ALL)
    log("DEBUG", tag, format, args...);
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
  bool _connected = false;
  int _sock;
  struct sockaddr_in _saddr = {0};
  String _fullAddr;

  const char* _name = "no_name";

  void connectSocket();
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
};

extern BetterLogger LOGGER;

#endif