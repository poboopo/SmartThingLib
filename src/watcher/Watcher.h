#ifndef WATCHER_H
#define WATCHER_H

#include <functional>
#include <ArduinoJson.h>
#include "watcher/callback/WatcherCallback.h"
#include "utils/List.h"
#include "logs/BetterLogger.h"
#include "configurable/ConfigurableObjects.h"

#define WATCHER_INFO_DOC_SIZE 128
#define WATCHER_TAG "watcher"

/*
    Класс наблюдатель за объектами
    T - тип данных, которые хранит в себе объект
*/

template<typename T>
class Watcher {
    public:
        Watcher(const Configurable::ConfigurableObject<T> * observable, T initialValue): 
            _observable(observable), _oldValue(initialValue), _callbackIdSequence(0) {};

        virtual bool check() = 0;
        virtual const char * getObservableInfo() = 0;

        DynamicJsonDocument getCallbacksJson() {
            return getCallbacksJson(false, false);
        }

        DynamicJsonDocument getCallbacksJson(bool ignoreReadOnly, bool shortJson) {
            if (_callbacks.size() == 0) {
                DynamicJsonDocument doc(1);
                return doc;
            }
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size());
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current) {
                if (current ==nullptr || 
                    ignoreReadOnly && current->isReadonly() ||
                    current->getId().isEmpty()
                ) {
                    return;
                }
                doc.add(current->toJson(shortJson));
            });
            return doc;
        };

        DynamicJsonDocument toJson(bool ignoreReadOnly, bool shortJson) {
            if (_callbacks.size() == 0) {
                DynamicJsonDocument doc(1);
                return doc;
            }
            DynamicJsonDocument callbacks = getCallbacksJson(ignoreReadOnly, shortJson);
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size() + 128);
            doc["observable"] = ((Configurable::ConfigurableObject<T> *) _observable)->toJson();
            doc["callbacks"] = callbacks;
            return doc;
        }
        
        String addCallback(Callback::WatcherCallback<T> * callback) {
            if (callback != nullptr) {
                if (callback->isReadonly()) {
                    callback->setId("system");
                }
                if (callback->getId().isEmpty() 
                    || callback->getId().equals("New")
                ) {
                    const char * id = getNextCallbackId();
                    if (id == nullptr) {
                        LOGGER.error(WATCHER_TAG, "Failed to generate new id for callback");
                        return "";
                    }
                    LOGGER.debug(WATCHER_TAG, "Generated new callback id: %s", id);
                    callback->setId(id);
                } else if (getCallbackById(callback->getId()) != nullptr) {
                    LOGGER.error(WATCHER_TAG, "Callback with id=%s already exists!", callback->getId().c_str());
                    return "";
                }
                _callbacks.append(callback);
                return callback->getId();
            }
            return "";
        };

        bool removeCallback(String id) {
            if (id.isEmpty()) {
                LOGGER.error(WATCHER_TAG, "Failed to remove callback - id is missing!");
                return false;
            }
            Callback::WatcherCallback<T> * callback = getCallbackById(id);
            if (callback == nullptr) {
                LOGGER.error(WATCHER_TAG, "Failed to remove callback - can't find callback with id %s", id);
                return false;
            }
            if (callback->isReadonly()) {
                LOGGER.error(WATCHER_TAG, "This callback is readonly!");
                return false;
            }
            if (_callbacks.remove(callback)) {
                delete(callback);
                return true;
            }
            LOGGER.error(WATCHER_TAG, "Failed to remove callback from list");
            return false;
        }

        Callback::WatcherCallback<T> * getCallbackById(String id) {
            if (id.isEmpty()) {
                return nullptr;
            }
            return _callbacks.findValue([&](Callback::WatcherCallback<T> * callback) {
                return callback->getId().equals(id);
            });
        }

        void callCallbacks(T &value) {
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current) {
                if (current->accept(value)) {
                    LOGGER.debug(WATCHER_TAG , "Calling callback [id=%s]", current->getId());
                    current->call(value);
                }
            });
        };
        
        const Configurable::ConfigurableObject<T> * getObservable() {
            return _observable;
        };

        bool haveCallbacks() {
            return _callbacks.size() != 0;
        }

        uint8_t callbacksCount() {
            return _callbacks.size();
        }
    protected:
        const  Configurable::ConfigurableObject<T> * _observable;
        T _oldValue;
        List<Callback::WatcherCallback<T>> _callbacks;
    private:
        uint16_t _callbackIdSequence;

        const char * getNextCallbackId() {
            char * buff = new char[256];
            bool res = false;
            uint8_t attempts = 20;
            while (!res && attempts != 0) {
                sprintf(buff, "%s_%s_%u\0", _observable->type, _observable->name, _callbackIdSequence);
                _callbackIdSequence++;
                res = getCallbackById(buff) == nullptr;
                attempts--;
            }
            if (attempts == 0) {
                return nullptr;
            }
            return buff;
        }
};
#endif