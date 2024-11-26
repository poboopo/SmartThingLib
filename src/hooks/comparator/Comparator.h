
#ifndef COMPARATOR_H
#define COMPARATOR_H
#include "logs/BetterLogger.h"

static const char * COMPARE_EQ = "eq";
static const char * COMPARE_NEQ = "neq";
static const char * COMPARE_GTE = "gte";
static const char * COMPARE_LTE = "lte";

enum CompareType { UNKNOWN_COMPARE, EQ, NEQ, GTE, LTE };

inline String compareTypeToString(CompareType type) {
  switch (type) {
    case EQ:
      return COMPARE_EQ;
    case NEQ:
      return COMPARE_NEQ;
    case GTE:
      return COMPARE_GTE;
    case LTE:
      return COMPARE_LTE;
    default:
      return "";
  }
}

inline CompareType compareTypeFromString(const char * type, CompareType defValue = UNKNOWN_COMPARE) {
  if (type == nullptr || strlen(type) == 0) {
    return defValue;
  }
  if (strcmp(type, COMPARE_EQ) == 0) {
    return CompareType::EQ;
  }
  if (strcmp(type, COMPARE_NEQ) == 0) {
    return CompareType::NEQ;
  }
  if (strcmp(type, COMPARE_GTE) == 0) {
    return CompareType::GTE;
  }
  if (strcmp(type, COMPARE_LTE) == 0) {
    return CompareType::LTE;
  }
  return defValue;
}
#endif