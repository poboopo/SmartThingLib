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
                HttpCallback(const char * url): _url(url) {};
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
                        int responseCode = client.GET();
                        LOGGER.info(HTTP_CALLBACK_TAG, "Request %s finished with code %d", _url, responseCode);
                        client.end();
                    } else {
                        LOGGER.error(HTTP_CALLBACK_TAG, "WiFi not connected!");
                    }
                }
            private:
                const char * _url;
        };
    }
}
#endif