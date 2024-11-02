
#ifndef COMPARATOR_H
#define COMPARATOR_H
#include "logs/BetterLogger.h"

enum CompareType { EQ, NEQ, GTE, LTE };

inline String compareTypeToString(CompareType type) {
  switch (type) {
    case EQ:
      return "eq";
    case NEQ:
      return "neq";
    case GTE:
      return "gte";
    case LTE:
      return "lte";
    default:
      return "";
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
  if (type.equals("lte")) {
    return CompareType::LTE;
  }
  return defValue;
}
#endif