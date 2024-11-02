#ifndef BetterLogger_H
#define BetterLogger_H

#include "Features.h"
#include <Arduino.h>

#if LOGGER_TYPE == MULTICAST_LOGGER
#include <WiFiUdp.h>
#endif
#if LOGGER_TYPE == TCP_LOGGER
#include <WiFiClient.h>
#endif

#if ENABLE_LOGGER && (LOGGING_LEVEL == LOGGING_LEVEL_DEBUG)
#define st_log_debug(tag, format, ...) LOGGER.debug(tag, format, ##__VA_ARGS__)
#define st_log_request(tag, method, uri, body) LOGGER.logRequest(tag, method, uri, body)
#else
#define st_log_debug(tag, format, ...)
#define st_log_request(tag, method, uri, body)
#endif

#if ENABLE_LOGGER && (LOGGING_LEVEL <= LOGGING_LEVEL_INFO)
#define st_log_info(tag, format, ...) LOGGER.info(tag, format, ##__VA_ARGS__)
#else
#define st_log_info(tag, format, ...)
#endif

#if ENABLE_LOGGER && (LOGGING_LEVEL <= LOGGING_LEVEL_WARN)
#define st_log_warning(tag, format, ...) LOGGER.warning(tag, format, ##__VA_ARGS__)
#else
#define st_log_warning(tag, format, ...)
#endif

#if ENABLE_LOGGER && (LOGGING_LEVEL <= LOGGING_LEVEL_ERROR)
#define st_log_error(tag, format, ...) LOGGER.error(tag, format, ##__VA_ARGS__)
#else
#define st_log_error(tag, format, ...)
#endif

static const char * LOGGER_TAG = "logger";

class BetterLogger {
 public:
  BetterLogger() {
    Serial.begin(115200);
  };
  ~BetterLogger() { 
    #if LOGGER_TYPE != SERIAL_LOGGER
    disconnect();
    #endif
  }

  void init(String fullAddr, const char name[]) {
    _name = name;
    #if LOGGER_TYPE != SERIAL_LOGGER
    _fullAddr = fullAddr;

    parseAddressAndConnect();
    #endif
  }

  void updateName(const char name[]) {
    _name = name;
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
    warning(LOGGER_TAG, "Server log address was updated to %s", fullAddr.c_str());
    parseAddressAndConnect();
    #endif
  }

  #if ENABLE_LOGGER
  template <typename... Args>
  void log(uint8_t level, const char* tag, const char* format, Args... args) {
    #if LOGGER_TYPE != SERIAL_LOGGER
    if (_connected) {
      if (sendRemote(level, tag, format, args...) <= 0) {
        _connected = false;
      }
      return;
    }
    #endif
    Serial.printf("[%s][%u][%s]", _name, level, tag);
    Serial.printf(format, args...);
    Serial.println();
  }
  #else
  template <typename... Args>
  void log(uint8_t level, const char* tag, const char* format, Args... args) {
  }
  #endif

  // bad impl, but i have no other ideas
  #if ENABLE_LOGGER && (LOGGING_LEVEL <= LOGGING_LEVEL_ERROR)
  template <typename... Args>
  void error(const char* tag, const char* format, Args... args) {
    log(LOGGING_LEVEL_ERROR, tag, format, args...);
  }
  #else
  template <typename... Args>
  void error(const char* tag, const char* format, Args... args) {}
  #endif

  #if ENABLE_LOGGER && (LOGGING_LEVEL <= LOGGING_LEVEL_WARN)
  template <typename... Args>
  void warning(const char* tag, const char* format, Args... args) {
    log(LOGGING_LEVEL_WARN, tag, format, args...);
  }
  #else
  template <typename... Args>
  void warning(const char* tag, const char* format, Args... args) {}
  #endif

  #if ENABLE_LOGGER && (LOGGING_LEVEL <= LOGGING_LEVEL_INFO)
  template <typename... Args>
  void info(const char* tag, const char* format, Args... args) {
    log(LOGGING_LEVEL_INFO, tag, format, args...);
  };
  #else
  template <typename... Args>
  void info(const char* tag, const char* format, Args... args) {}
  #endif

  #if ENABLE_LOGGER && (LOGGING_LEVEL == LOGGING_LEVEL_DEBUG)
  template <typename... Args>
  void debug(const char* tag, const char* format, Args... args) {
    log(LOGGING_LEVEL_DEBUG, tag, format, args...);
  };
  #else
  template <typename... Args>
  void debug(const char* tag, const char* format, Args... args) {}
  #endif

  void logRequest(const char* tag, const char * method, const char* uri, const char* body) {
    debug(tag, "[%s] %s - %s", method, uri, body == nullptr ? "" : body);
  };

 private:
  const char* _name = "no_name";
  #if LOGGER_TYPE == TCP_LOGGER || LOGGER_TYPE == MULTICAST_LOGGER
  bool _connected = false;
  String _fullAddr;

  #if LOGGER_TYPE == MULTICAST_LOGGER
  WiFiUDP _udp;
  String _ip;
  #endif
  #if LOGGER_TYPE == TCP_LOGGER
  WiFiClient _tcp;
  #endif

  template <typename... Args>
  size_t sendRemote(uint8_t level, const char* tag, const char* format, Args... args) {
    size_t size = 0;
    #if LOGGER_TYPE == TCP_LOGGER
    // name_&_level_&_tag_&_message
    size += _tcp.printf("%s&%u&%s&", _name, level, tag);
    size += _tcp.printf(format, args...);
    _tcp.println();
    return size;
    #endif
    #if LOGGER_TYPE == MULTICAST_LOGGER
    _udp.beginMulticastPacket();
    // ip_name_&_level_&_tag_&_message
    size += _udp.printf("%s&%s&%u&%s&", _ip.c_str(), _name, level, tag);
    size += _udp.printf(format, args...);
    _udp.println();
    _udp.endPacket();
    #endif
    return size;
  }

  bool connect(const char * ip, int port) {
    IPAddress address;
    address.fromString(ip);
    #if LOGGER_TYPE == TCP_LOGGER
    _connected = _tcp.connect(address, port);
    #endif
    #if LOGGER_TYPE == MULTICAST_LOGGER
    _connected = _udp.beginMulticast(address, port);
    if (_connected) {
      _ip = WiFi.localIP();
    }
    #endif
    return _connected;
  }

  void disconnect() {
    if (!_connected) {
      return;
    }
    #if LOGGER_TYPE == TCP_LOGGER
    _tcp.stop();
    #endif
    #if LOGGER_TYPE == MULTICAST_LOGGER
    _udp.stop();
    #endif
    _connected = false;
  }

  void parseAddressAndConnect() {
    if (_fullAddr.isEmpty() || _fullAddr.equals("null")) {
      if (_connected) {
        warning(LOGGER_TAG, "Server info was deleted, disconnecting");
        disconnect();
      }
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
    if (connect(ip.c_str(), port.toInt())) {
      Serial.println();
      Serial.println("Remote logger connected! Serial output disabled while remote logger connected!");
      info(LOGGER_TAG, "Logger connected!");
    } else {
      error(LOGGER_TAG, "Failed to connect");
    }
  };
  #endif
};

extern BetterLogger LOGGER;

#endif