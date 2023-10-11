
#ifndef COMPARATOR_H
#define COMPARATOR_H

namespace Callback {
    enum CompareType {
        EQ,
        GTE,
        LTE
    };

    inline String compareTypeToString(CompareType type) {
        switch(type) {
            case EQ:
                return "eq";
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
                    case EQ:
                        return eq(value, triggerValue);
                    case GTE:
                        return gte(value, triggerValue);
                    case LTE:
                        return lte(value, triggerValue);
                }
                return false;
            }

            static bool isValidTrigger(int16_t triggerValue) {
                return triggerValue > -1;
            }
            static bool eq(int16_t &value, int16_t &triggerValue) {
                return value == triggerValue;
            }
            static bool gte(int16_t &value, int16_t &triggerValue) {
                return value >= triggerValue;
            }
            static bool lte(int16_t &value, int16_t &triggerValue) {
                return value <= triggerValue;
            }

            static bool isValidTrigger(String triggerValue) {
                return !triggerValue.isEmpty();
            }
            static bool eq(String &value, String &triggerValue) {
                return value.equals(triggerValue);
            }
            static bool gte(String &value, String &triggerValue) {
                return value.length() >= triggerValue.length();
            }
            static bool lte(String &value, String &triggerValue) {
                return value.length() <= triggerValue.length();
            }
    };
}
#endif