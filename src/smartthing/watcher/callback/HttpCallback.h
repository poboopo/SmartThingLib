#ifndef HTTP_CALLBACK_H
#define HTTP_CALLBACK_H

#include <HTTPClient.h>

#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/logs/BetterLogger.h"

#define HTTP_CALLBACK_TAG "http_callback"

namespace Watcher {
    namespace Callback {
        template<typename T>
        class HttpCallback: public WatcherCallback<T> {
            public:
                HttpCallback(const char * url, T triggerValue):
                    WatcherCallback<T>(triggerValue), _url(url) {};
                void call(T * value) {
                    if (value == nullptr) {
                        LOGGER.error(HTTP_CALLBACK_TAG, "Value is null!");
                        return;
                    }
                    // todo create async task
                    // todo replace ${value} in url
                    // add value generator lambda?
                    if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
                        HTTPClient client;
                        client.begin(_url);
                        _lastResponseCode = client.GET();
                        LOGGER.info(HTTP_CALLBACK_TAG, "Request %s finished with code %d", _url, _lastResponseCode);
                        client.end();
                    } else {
                        LOGGER.error(HTTP_CALLBACK_TAG, "WiFi not connected!");
                    }
                };

                StaticJsonDocument<INFO_DOC_SIZE> getInfo() {
                    StaticJsonDocument<INFO_DOC_SIZE> doc;
                    doc["type"] = HTTP_CALLBACK_TAG;
                    doc["url"] = _url;
                    doc["lastResponseCode"] = _lastResponseCode;
                    return doc;
                };
            private:
                const char * _url;
                uint16_t _lastResponseCode = 0;
        };
    }
}
#endif