#ifndef DEVICE_STATE_WATCHER_H
#define DEVICE_STATE_WATCHER_H

#include "smartthing/watcher/Watcher.h"
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/configurable/ConfigurableObjects.h"
#include "smartthing/logs/BetterLogger.h"
#include "smartthing/watcher/callback/WatcherCallback.h"

#define DEVICE_STATE_WATCHER_TAG "device_state_watcher"

using namespace Configurable::Sensor;
#define STATE_WATCHER_TYPE DEVICE_STATE_WATCHER_TAG

namespace Watcher {
    class DeviceStateWatcher: public Watcher<Configurable::DeviceState::DeviceState, char *> {
        public:
            DeviceStateWatcher(const Configurable::DeviceState::DeviceState * deviceState, Callback::WatcherCallback<char *> * callback): 
                Watcher<Configurable::DeviceState::DeviceState, char *>(deviceState, callback) {
                    _oldValue = nullptr;
                };
            bool check() {
                if (_observable == nullptr) {
                    return false;
                }

                const char * newValue = _observable->valueGenerator();
                if (_oldValue == nullptr) {
                    copyValue(newValue);
                    return false;
                }
                
                if (strcmp(newValue, _oldValue) != 0) {
                    LOGGER.debug(
                        DEVICE_STATE_WATCHER_TAG, 
                        "Device state %s value changed %s->%s.", 
                        _observable->name, _oldValue, newValue
                    );
                    copyValue(newValue);

                    callCallbacks(&_oldValue);
                    return true;
                }
                return false;
            };

            bool callbackAccept(Callback::WatcherCallback<char *> * callback, char ** value) {
                return callback->triggerValue() == nullptr || 
                    (callback->triggerValue() != nullptr && strcmp((*value), callback->triggerValue()) != 0);
            }

            StaticJsonDocument<WATCHERS_CALLBACK_INFO_DOC_SIZE> getInfo() {
                StaticJsonDocument<WATCHERS_CALLBACK_INFO_DOC_SIZE> doc;
                doc["type"] = STATE_WATCHER_TYPE;
                doc["observable"] = _observable->name;
                return doc;
            };
            
            const void * getObservable() {
                return (void *) _observable;
            }
        private:
            void copyValue(const char * value) {
                if (_oldValue != nullptr) {
                    delete(_oldValue);
                }
                int size = strlen(value) + 1;
                _oldValue = new char[size];
                strncpy(_oldValue, value, size);
                _oldValue[size - 1] = '\0';
            }
    };
}

#endif