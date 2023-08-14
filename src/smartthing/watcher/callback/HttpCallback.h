#ifndef HTTP_CALLBACK_H
#define HTTP_CALLBACK_H

#include <HTTPClient.h>

#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/logs/BetterLogger.h"

#define HTTP_CALLBACK_TAG "http_callback"

namespace Callback {
    template<typename T>
    class HttpCallback: public WatcherCallback<T> {
        public:
            HttpCallback(const char * url, T triggerValue, bool readonly):
                WatcherCallback<T>(HTTP_CALLBACK_TAG, triggerValue, readonly), _url(url), _method("GET") {
                    fixUrl();
                };
            HttpCallback(const char * url, const char * method, const char * payload, T triggerValue, bool readonly):
                WatcherCallback<T>(HTTP_CALLBACK_TAG, triggerValue, readonly), _url(url), _method(method), _payload(payload) {
                    fixUrl();
                    if (_method.isEmpty()) {
                        _method = "GET";
                    }
                };
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
                doc["caption"] = "http";
                doc["url"] = _url;
                doc["method"] = _method;
                doc["payload"] = _payload;
                doc["lastResponseCode"] = _lastResponseCode;
                return doc;
            };

            void updateCustom(DynamicJsonDocument doc) {
                if (doc.containsKey("url")) {
                    _url = doc["url"].as<String>();
                    fixUrl();
                    LOGGER.debug(HTTP_CALLBACK_TAG, "Callback's url was updated to %s", _url.c_str());
                }
                if (doc.containsKey("method")) {
                    _method = doc["method"].as<String>();
                    LOGGER.debug(HTTP_CALLBACK_TAG, "Callback's method was updated to %s", _url.c_str());
                }
                if (doc.containsKey("payload")) {
                    _payload = doc["payload"].as<String>();
                    LOGGER.debug(HTTP_CALLBACK_TAG, "Callback's payload was updated to %s", _url.c_str());
                }
            };

            TaskHandle_t _requestTask = NULL; // not safe
            bool _sending = false;
        private:
            String _url;
            String _method = "GET";
            String _payload;
            int16_t _lastResponseCode = 0;

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
                    HTTP_CALLBACK_TAG,
                    10000,
                    this,
                    1,
                    &_requestTask
                );
            }

            void sendRequest() {
                HTTPClient client;
                LOGGER.info(HTTP_CALLBACK_TAG, "Sending request to %s", _url.c_str());
                client.setTimeout(2000);
                client.begin(_url);
                _lastResponseCode = client.sendRequest(_method.c_str(), _payload.c_str());
                LOGGER.info(HTTP_CALLBACK_TAG, "Request %s finished with code %d", _url.c_str(), _lastResponseCode);
                client.end();
            }

            void fixUrl() {
                if (!_url.startsWith("http")) {
                    _url = "http://" + _url;
                }
            }
    };
}
#endif