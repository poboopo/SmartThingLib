#ifndef WATCHER_H
#define WATCHER_H

#include <functional>
#include <ArduinoJson.h>
#include "smartthing/watcher/callback/WatcherCallback.h"
#include "smartthing/utils/List.h"

#define WATCHERS_CALLBACK_INFO_DOC_SIZE 128

/*
    Класс наблюдатель за объектами
*/

// O - класс наблюдаемого объекта
// T - тип данных, которые хранит в себе объект
template<typename O, typename T>
class Watcher {
    public:
        Watcher(const O * observable, Callback::WatcherCallback<T> * callback): 
            _observable(observable), _callbacks(callback) {};

        virtual bool check() = 0;
        virtual const char * getObservableInfo() = 0;
        // это находится здесь, тк у callback нет привязки к типу,а у Watcherов есть
        virtual bool callbackAccept(Callback::WatcherCallback<T> * callback, T * newValue) = 0;

        DynamicJsonDocument getCallbacksJson() {
            DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE * _callbacks.size());
            _callbacks.forEach([&](Callback::WatcherCallback<T> * current) {
                doc.add(current->toJson());
            });
            return doc;
        };
        
        void addCallback(Callback::WatcherCallback<T> * callback) {
            if (callback != nullptr) {
                _callbacks.append(callback);
            }
        };

        void callCallbacks(T * value) {
            _callbacks.forEach([this, value](Callback::WatcherCallback<T> * current) {
                if (callbackAccept(current, value)) {
                    current->call(value);
                }
            });
        };
        
        const O * getObservable() {
            return _observable;
        };
    protected:
        const O * _observable;
        T _oldValue;
        List<Callback::WatcherCallback<T>> _callbacks;
};
#endif