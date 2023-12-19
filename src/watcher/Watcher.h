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

        DynamicJsonDocument getObservableCallbacksJson() {
            return getObservableCallbacksJson(false, false);
        }

        DynamicJsonDocument getObservableCallbacksJson(bool ignoreReadOnly, bool shortJson) {
            if (_callbacks.size() == 0) {
                DynamicJsonDocument doc(0);
                doc.to<JsonArray>();
                return doc;
            }
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size());
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current) {
                if (current ==nullptr || 
                    ignoreReadOnly && current->isReadonly() ||
                    current->getId() < 0
                ) {
                    return;
                }
                doc.add(current->toJson(shortJson));
            });
            return doc;
        };

        DynamicJsonDocument toJson(bool ignoreReadOnly, bool shortJson) {
            if (_callbacks.size() == 0) {
                DynamicJsonDocument doc(0);
                return doc;
            }
            DynamicJsonDocument callbacks = getObservableCallbacksJson(ignoreReadOnly, shortJson);
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size() + 128);
            doc["observable"] = ((Configurable::ConfigurableObject<T> *) _observable)->toJson();
            doc["callbacks"] = callbacks;
            return doc;
        }
        
        bool addCallback(Callback::WatcherCallback<T> * callback) {
            if (callback == nullptr) {
                LOGGER.error(WATCHER_TAG, "Callback is missing!");
                return false;
            }

            if (callback->isReadonly()) {
                LOGGER.debug(WATCHER_TAG, "Callback is readonly, skipping id generation");
                callback->setId(-1);
            } else if (callback->getId() < 0) {
                int id = getNextCallbackId();
                if (id < 0) {
                    LOGGER.error(WATCHER_TAG, "Failed to generate new id for callback");
                    return false;
                }
                LOGGER.debug(WATCHER_TAG, "Generated new callback id=%d", id);
                callback->setId(id);
            } else if (getCallbackById(callback->getId()) != nullptr) {
                LOGGER.error(WATCHER_TAG, "Callback with id=%d already exists!", callback->getId());
                return false;
            }
            
            _callbacks.append(callback);
            LOGGER.debug(WATCHER_TAG, "New callback added id=%d", callback->getId());
            return true;
        };

        bool removeCallback(int id) {
            if (id < 0) {
                LOGGER.error(WATCHER_TAG, "Failed to remove callback - id negative!");
                return false;
            }
            Callback::WatcherCallback<T> * callback = getCallbackById(id);
            if (callback == nullptr) {
                LOGGER.error(WATCHER_TAG, "Failed to remove callback - can't find callback with id %d", id);
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

        Callback::WatcherCallback<T> * getCallbackById(int id) {
            if (id < 0) {
                return nullptr;
            }
            return _callbacks.findValue([&](Callback::WatcherCallback<T> * callback) {
                return callback->getId() == id;
            });
        }

        void callCallbacks(T &value) {
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current) {
                if (current->accept(value)) {
                    LOGGER.debug(WATCHER_TAG , "Calling callback [id=%d]", current->getId());
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
        int _callbackIdSequence;

        int getNextCallbackId() {
            bool res = false;
            uint8_t attempts = 20;
            while (!res && attempts != 0) {
                _callbackIdSequence++;
                res = getCallbackById(_callbackIdSequence) == nullptr;
                attempts--;
            }
            if (attempts == 0) {
                return -1;
            }
            return _callbackIdSequence;
        }
};
#endif