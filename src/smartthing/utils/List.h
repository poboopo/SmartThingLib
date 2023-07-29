#ifndef MY_LIST_H
#define MY_LIST_H

#include <Arduino.h>
#include <functional>

template<typename T>
class List {
    public:
        typedef std::function<void(T *)> ForEachFunction;
        typedef std::function<bool(T *)> FindFunction;

        List():_count(0), _head(nullptr){};
        List(T * value):_head(new Wrapper(value)), _count(1){};
        ~List() {
            removeAll();
        };

        void removeAll() {
            Wrapper * current = _head;
            while (current->next != nullptr) {
                current = current->next;
                delete(current->value);
                delete(current->previous);
            }
            delete(current);
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

        void remove(T * value) {
            if (value == nullptr) {
                return;
            }

            Wrapper * current = _head;
            while (current != nullptr) {
                if (value == current->value) {
                    if (current->previous != nullptr) {
                        current->previous->next = current->next;
                    }
                    if (current->next != nullptr) {
                        current->next->previous = current->next;
                    }
                    delete(current);
                    return;
                }
                current = current->next;
            }
        };

        void forEach(ForEachFunction forFunc) {
            Wrapper * current = _head;
            while (current != nullptr) {
                forFunc(current->value);
                current = current->next;
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