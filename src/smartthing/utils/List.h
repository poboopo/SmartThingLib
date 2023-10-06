#ifndef MY_LIST_H
#define MY_LIST_H

#include <Arduino.h>
#include <functional>
#include "smartthing/logs/BetterLogger.h"

template<typename T>
class List {
    public:
        typedef std::function<void(T *)> ForEachFunction;
        typedef std::function<void(T *, int index)> ForEachIndexFunction;
        typedef std::function<bool(T *)> FindFunction;

        List():_count(0), _head(nullptr){};
        List(T * value):_head(new Wrapper(value)), _count(1){};
        ~List() {
            removeAll();
        };

        void removeAll() {
            if (_count == 0) {
                return;
            }
            Wrapper * current = _head;
            Wrapper * temp = new Wrapper(nullptr);
            while (current != nullptr) {
                temp = current;
                current = current->next;
                delete(temp);
            }
        }

        int16_t append(T * value) {
            if (value == nullptr) {
                return -1;
            }

            Wrapper * wrap = new Wrapper(value);
            wrap->next = _head;
            if (_head != nullptr) {
                _head->previous = wrap;
            }
            _head = wrap;
            _count++;
            return _count - 1;
        };

        bool remove(T * value) {
            if (value == nullptr || _count == 0) {
                return false;
            }

            Wrapper * current = _head;
            while (current != nullptr) {
                if (value == current->value) {
                    if (current->next != nullptr) {
                        current->next->previous = current->previous;
                        if (current->previous == nullptr) {
                            _head = current->next;
                        }
                    }
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    }
                    delete(current);
                    _count--;
                    return true;
                }
                current = current->next;
            }
            return false;
        };

        void forEach(ForEachFunction forFunc) {
            Wrapper * current = _head;
            while (current != nullptr) {
                forFunc(current->value);
                current = current->next;
            }
        };

        void forEach(ForEachIndexFunction forFunc) {
            Wrapper * current = _head;
            int i = 0;
            while (current != nullptr) {
                forFunc(current->value, i);
                current = current->next;
                i++;
            }
        };

// todo const?
        T * findValue(FindFunction findFunc) {
            Wrapper * current = _head;
            while (current != nullptr) {
                if (findFunc(current->value)) {
                    return current->value;
                }
                current = current->next;
            }
            return nullptr;
        };

        int16_t indexOf(T * value) {
            if (_count == 0) {
                return -1;
            }
            int16_t i = 0;
            Wrapper * current = _head;
            while (current != nullptr) {
                if (current->value == value) {
                    return i;
                }
                i++;
                current = current->next;
            }
            return -1;
        };

        T * getByIndex(int16_t index) {
            if (_count == 0 || index < 0 || index > _count) {
                return nullptr;
            }
            int16_t i = 0;
            Wrapper * current = _head;
            while (current != nullptr) {
                if (i == index) {
                    return current->value;
                }
                i++;
                current = current->next;
            }
            return nullptr;
        }

        int16_t size() {
            return _count;
        }
    private:
        struct Wrapper {
            Wrapper(T * v): value(v), next(nullptr), previous(nullptr) {};
            T * value;
            Wrapper * next;
            Wrapper * previous;
        };

        Wrapper * _head;
        int16_t _count;
};

#endif