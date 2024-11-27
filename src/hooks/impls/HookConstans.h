#ifndef HOOK_CONSTANTS_H
#define HOOK_CONSTANTS_H

#include <Arduino.h>

static const char * _lambdaHookType = "lambda";
static const char * _actionHookType = "action";
static const char * _httpHookType = "http";
static const char * _notificationHookType = "notification";

static const char * _idHookField = "id";
static const char * _readonlyHookField = "readonly";
static const char * _typeHookField = "type";
static const char * _triggerEnabledHookField = "triggerEnabled";
static const char * _triggerHookField = "trigger";
static const char * _compareTypeHookField = "compareType";
static const char * _thresholdHookField = "threshold";

static const char * COMPARE_EQ = "eq";
static const char * COMPARE_NEQ = "neq";
static const char * COMPARE_GTE = "gte";
static const char * COMPARE_LTE = "lte";

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