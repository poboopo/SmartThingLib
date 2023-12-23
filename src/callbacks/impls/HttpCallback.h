#ifndef HTTP_CALLBACK_H
#define HTTP_CALLBACK_H

#include <HTTPClient.h>

#include "callbacks/impls/WatcherCallback.h"
#include "logs/BetterLogger.h"

#define HTTP_CALLBACK_TAG "http_callback"
#define VALUE_DYNAMIC_PARAM "${v}"

namespace Callback {
    const String HTTP_CALLBACKS_TEMPLATES_JSON = R"=====(
    {
        "url": {
            "required": true
        },
        "method": {
            "required": false,
            "values": [
                "GET",
                "POST",
                "DELETE",
                "PUT"
            ]
        },
        "payload": {
            "required": false
        }
    }
    )=====";

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
            void call(T &value) {
                _currentValue = value;
                if (WiFi.isConnected() || WiFi.getMode() == WIFI_MODE_AP) {
                    createRequestTask();
                } else {
                    LOGGER.error(HTTP_CALLBACK_TAG, "WiFi not connected!");
                }
            };

            DynamicJsonDocument toJson(bool shortJson) {
                DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE);
                if (!shortJson) {
                    doc["lastResponseCode"] = _lastResponseCode;
                }
                doc["url"] = _url;
                doc["method"] = _method;
                doc["payload"] = _payload;
                this->addDefaultInfo(doc);
                return doc;
            };

            void updateCustom(JsonObject doc) {
                if (doc.containsKey("url")) {
                    _url = doc["url"].as<String>();
                    fixUrl();
                    LOGGER.debug(HTTP_CALLBACK_TAG, "Callback's url was updated to %s", _url.c_str());
                }
                if (doc.containsKey("method")) {
                    _method = doc["method"].as<String>();
                    LOGGER.debug(HTTP_CALLBACK_TAG, "Callback's method was updated to %s", _method.c_str());
                }
                if (doc.containsKey("payload")) {
                    _payload = doc["payload"].as<String>();
                    LOGGER.debug(HTTP_CALLBACK_TAG, "Callback's payload was updated to %s", _payload.c_str());
                }
            };

            static DynamicJsonDocument getTemplate() {
                DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
                deserializeJson(doc, HTTP_CALLBACKS_TEMPLATES_JSON);
                return doc;
            }

            TaskHandle_t _requestTask = NULL; // not safe
            bool _sending = false;
        private:
            String _url;
            String _method;
            String _payload;
            int16_t _lastResponseCode = 0;
            T _currentValue;

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
                    _url.c_str(),
                    10000,
                    this,
                    1,
                    &_requestTask
                );
            }

            void sendRequest() {
                //probably bad realisation for memory

                String valueStr = String(_currentValue);

                String urlCopy = _url;
                String payloadCopy = _payload;
                
                LOGGER.debug(HTTP_CALLBACK_TAG, "Replacing ${v} with [%s]", valueStr.isEmpty() ? "empty value" : valueStr.c_str());
                urlCopy.replace(VALUE_DYNAMIC_PARAM, valueStr);
                payloadCopy.replace(VALUE_DYNAMIC_PARAM, valueStr);
                LOGGER.info(HTTP_CALLBACK_TAG, "Sending request [%s] %s[%s] :: %s", _method.c_str(), _url.c_str(), urlCopy.c_str(), payloadCopy.c_str());
                
                HTTPClient client;
                client.setTimeout(2000);
                client.begin(urlCopy);
                _lastResponseCode = client.sendRequest(_method.c_str(), payloadCopy.c_str());
                client.end();
                
                LOGGER.info(HTTP_CALLBACK_TAG, "Request %s finished with code %d", urlCopy.c_str(), _lastResponseCode);
            }

            void sendSimpleRequest() {
                LOGGER.info(HTTP_CALLBACK_TAG, "Sending simple request [%s] %s :: %s", _method.c_str(), _url.c_str(), _payload.c_str());
                
                HTTPClient client;
                client.setTimeout(2000);
                client.begin(_url);
                _lastResponseCode = client.sendRequest(_method.c_str(), _payload.c_str());
                client.end();
                
                LOGGER.info(HTTP_CALLBACK_TAG, "Request [%s] finished with code [%d]", _url.c_str(), _lastResponseCode);
            }

            void fixUrl() {
                if (!_url.startsWith("http")) {
                    _url = "http://" + _url;
                }
            }
    };
}
#endif