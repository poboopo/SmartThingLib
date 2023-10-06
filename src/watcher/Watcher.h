#ifndef WATCHER_H
#define WATCHER_H

#include <functional>
#include <ArduinoJson.h>
#include "watcher/callback/WatcherCallback.h"
#include "utils/List.h"
#include "logs/BetterLogger.h"
#include "configurable/ConfigurableObjects.h"

#define WATCHER_INFO_DOC_SIZE 128

/*
    Класс наблюдатель за объектами
*/

// T - тип данных, которые хранит в себе объект
// todo remove typename O and just use configurable object!
template<typename T>
class Watcher {
    public:
        Watcher(const Configurable::ConfigurableObject<T> * observable, Callback::WatcherCallback<T> * callback, T initialValue): 
            _observable(observable), _callbacks(callback), _oldValue(initialValue) {};

        virtual bool check() = 0;
        virtual const char * getObservableInfo() = 0;
        // это находится здесь, тк у callback нет привязки к типу,а у Watcherов есть
        virtual bool callbackAccept(Callback::WatcherCallback<T> * callback,  T &oldValue, T &newValue) = 0;

        DynamicJsonDocument getCallbacksJson() {
            return getCallbacksJson(false, false);
        }

        DynamicJsonDocument getCallbacksJson(bool ignoreReadOnly, bool shortJson) {
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size());
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current) {
                if (ignoreReadOnly && current->isReadonly()) {
                    return;
                }
                doc.add(current->toJson(shortJson));
            });
            return doc;
        };

        DynamicJsonDocument toJson(bool ignoreReadOnly, bool shortJson) {
            DynamicJsonDocument callbacks = getCallbacksJson(ignoreReadOnly, shortJson);
            if (callbacks.size() == 0) {
                return callbacks;
            }
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size() + 128);
            doc["observable"] = ((Configurable::ConfigurableObject<T> *) _observable)->toJson();
            doc["callbacks"] = callbacks;
            return doc;
        }
        
        void addCallback(Callback::WatcherCallback<T> * callback) {
            if (callback != nullptr) {
                _callbacks.append(callback);
            }
        };

        bool removeCallback(int16_t index) {
            Callback::WatcherCallback<T> * callback = _callbacks.getByIndex(index);
            if (callback == nullptr) {
                return false;
            }
            if (callback->isReadonly()) {
                LOGGER.error("watcher", "This callback is readonly!");
                return false;
            }
            if (_callbacks.remove(callback)) {
                delete(callback);
                return true;
            }
            return false;
        }

        Callback::WatcherCallback<T> * getCallback(int16_t index) {
            return _callbacks.getByIndex(index);
        }

        void callCallbacks(T &oldValue, T &value) {
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current, int index) {
                if (callbackAccept(current, oldValue, value)) {
                    current->call(&value);
                } else {
                    LOGGER.debug("watcher", "Callback reject [%d]:%s", index, current->type());
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
};
#endif