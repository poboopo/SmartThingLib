
#ifndef COMPARATOR_H
#define COMPARATOR_H
#include "logs/BetterLogger.h"

#define COMPARATOR_TAG "comparator"

namespace Callback {
    enum CompareType {
        EQ,
        NEQ,
        GTE,
        LTE
    };

    inline String compareTypeToString(CompareType type) {
        switch(type) {
            case EQ:
                return "eq";
            case NEQ:
                return "neq";
            case GTE:
                return "gte";
            case LTE:
                return "lte";
            default:
                return "error";
        }
    }

    inline CompareType compareTypeFromString(String type, CompareType defValue) {
        if (type.isEmpty()) {
            return defValue;
        }
        if (type.equals("eq")) {
            return CompareType::EQ;
        } 
        if (type.equals("neq")) {
            return CompareType::NEQ;
        } 
        if (type.equals("gte")) {
            return CompareType::GTE;
        } 
        if (type.equals("gte")) {
            return CompareType::LTE;
        }
        return defValue;
    }

    class Comparator {
        public:
            template<typename T>
            static bool compare(CompareType compareType, T &value, T &triggerValue) {
                switch (compareType) {
                    case CompareType::EQ:
                        return eq(value, triggerValue);
                    case CompareType::NEQ:
                        return neq(value, triggerValue);
                    case CompareType::GTE:
                        return gte(value, triggerValue);
                    case CompareType::LTE:
                        return lte(value, triggerValue);
                }
                return false;
            }

            static bool eq(int16_t &value, int16_t &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %d == %d", value, triggerValue);
                return value == triggerValue;
            }
            static bool neq(int16_t &value, int16_t &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %d != %d", value, triggerValue);
                return value != triggerValue;
            }
            static bool gte(int16_t &value, int16_t &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %d >= %d", value, triggerValue);
                return value >= triggerValue;
            }
            static bool lte(int16_t &value, int16_t &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %d <= %d", value, triggerValue);
                return value <= triggerValue;
            }

            static bool eq(String &value, String &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %s == %s", value, triggerValue);
                return value.equals(triggerValue);
            }
            static bool neq(String &value, String &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %s != %s", value, triggerValue);
                return !value.equals(triggerValue);
            }
            static bool gte(String &value, String &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %s >= %s", value, triggerValue);
                return value.length() >= triggerValue.length();
            }
            static bool lte(String &value, String &triggerValue) {
                LOGGER.debug(COMPARATOR_TAG, "Compare %s <= %s", value, triggerValue);
                return value.length() <= triggerValue.length();
            }
    };
}
#endif