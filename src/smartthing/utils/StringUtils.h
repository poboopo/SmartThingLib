#ifndef MY_STRING_UTILS_H
#define MY_STRING_UTILS_H

#include <Arduino.h>

inline char * copyString(const char * from) {
    if (from == nullptr) {
        return nullptr;
    }
    int size = strlen(from) + 1;
    char * result = new char[size];
    strncpy(result, from, size);
    result[size - 1] = '\0';
    return result;
}

inline void copyString(char * to, const char * from) {
    if (from == nullptr) {
        copyString(to, "empty");
        return;
    }
    int size = strlen(from) + 1;
    to = new char[size];
    strncpy(to, from, size);
    to[size - 1] = '\0';
}



#endif