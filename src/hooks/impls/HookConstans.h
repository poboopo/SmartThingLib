#ifndef HOOK_CONSTANTS_H
#define HOOK_CONSTANTS_H

#include <Arduino.h>

const char * const _lambdaHookType = "lambda";
const char * const _actionHookType = "action";
const char * const _httpHookType = "http";
const char * const _notificationHookType = "notification";

const char * const _idHookField = "id";
const char * const _readonlyHookField = "readonly";
const char * const _typeHookField = "type";
const char * const _triggerEnabledHookField = "triggerEnabled";
const char * const _triggerHookField = "trigger";
const char * const _compareTypeHookField = "compareType";
const char * const _thresholdHookField = "threshold";

const char * const COMPARE_EQ = "eq";
const char * const COMPARE_NEQ = "neq";
const char * const COMPARE_GTE = "gte";
const char * const COMPARE_LTE = "lte";

enum HookType {
  UNKNOWN_HOOK,
  LAMBDA_HOOK,
  ACTION_HOOK,
  HTTP_HOOK,
  NOTIFICATION_HOOK
};

enum CompareType { 
  UNKNOWN_COMPARE,
  EQ,
  NEQ,
  GTE,
  LTE
};

inline const char * hookTypeToStr(HookType type) {
  switch (type) {
    case LAMBDA_HOOK:
      return _lambdaHookType;
    case ACTION_HOOK:
      return _actionHookType;
    case HTTP_HOOK:
      return _httpHookType;
    case NOTIFICATION_HOOK:
      return _notificationHookType;
    default:
      return "unknown";
  }
}

inline HookType hookTypeFromStr(const char * type) {
  if (type == nullptr) {
    return UNKNOWN_HOOK;
  }
  if (strcmp(type, _actionHookType) == 0) {
    return ACTION_HOOK;
  }
  if (strcmp(type, _httpHookType) == 0) {
    return HTTP_HOOK;
  }
  if (strcmp(type, _notificationHookType) == 0) {
    return NOTIFICATION_HOOK;
  }
  if (strcmp(type, _lambdaHookType) == 0) {
    return LAMBDA_HOOK;
  }
  return UNKNOWN_HOOK;
}


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