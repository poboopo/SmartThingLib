#ifndef LAMBDA_HOOK_H
#define LAMBDA_HOOK_H

#include <functional>
#include <type_traits>

#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"

template<class T, CHECK_HOOK_DATA_TYPE>
class LambdaHook : public SELECT_HOOK_BASE_CLASS {
 public:
  typedef std::function<void(T &value)> CustomHook;

  LambdaHook(CustomHook customHook)
      : SELECT_HOOK_BASE_CLASS(LAMBDA_HOOK),
        _customHook(customHook){};
  virtual ~LambdaHook() {};

  void call(T &value) { _customHook(value); };

 private:
  CustomHook _customHook;
};
#endif