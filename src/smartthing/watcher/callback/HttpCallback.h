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
                HttpCallback(const char * url, T triggerValue, bool readonly):
                    WatcherCallback<T>(triggerValue, readonly), _url(url) {};
                void call(T * value) {
                    if (value == nullptr) {
                        LOGGER.error(HTTP_CALLBACK_TAG, "Value is null!");
                        return;
                    }
                    // todo replace ${value} in url
                    if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
                        createRequestTask();
                    } else {
                        LOGGER.error(HTTP_CALLBACK_TAG, "WiFi not connected!");
                    }
                };

                StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> toJson() {
                    StaticJsonDocument<CALLBACK_INFO_DOC_SIZE> doc = this->getDeaultInfo();
                    doc["type"] = HTTP_CALLBACK_TAG;
                    doc["caption"] = "http";
                    doc["url"] = _url;
                    doc["lastResponseCode"] = _lastResponseCode;
                    return doc;
                };

                TaskHandle_t _requestTask = NULL; // not safe
                bool _sending = false;
            private:
                const char * _url;
                uint16_t _lastResponseCode = 0;

                void createRequestTask() {
                    if (_sending) {
                        LOGGER.warning(HTTP_CALLBACK_TAG, "Request task already exist! Skipping");
                        return;
                    }
                    xTaskCreate(
                        [](void * o) {
                            HttpCallback * callback = static_cast<HttpCallback*>(o);
                            callback->_sending = true;
                            callback->sendRequest();
                            callback->_sending = false;
                            vTaskDelete(callback->_requestTask);
                        },
                        "http_callback",
                        10000,
                        this,
                        1,
                        &_requestTask
                    );
                }

                void sendRequest() {
                    HTTPClient client;
                    LOGGER.info(HTTP_CALLBACK_TAG, "Sending request to %s", _url);
                    client.setTimeout(2000);
                    client.begin(_url);
                    _lastResponseCode = client.GET();
                    LOGGER.info(HTTP_CALLBACK_TAG, "Request %s finished with code %d", _url, _lastResponseCode);
                    client.end();
                }
        };
    }
}
#endif