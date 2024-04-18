#ifndef LAMBDA_HOOK_H
#define LAMBDA_HOOK_H

#include <functional>
#include <type_traits>

#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"

#define LAMBDA_HOOK_TAG "lambda_hook"

namespace Hook {
template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
class LambdaHook : public T {
 public:
  typedef std::function<void(V &value)> CustomHook;

  LambdaHook(CustomHook customHook, bool readOnly)
      : T(LAMBDA_HOOK_TAG, readOnly),
        _customHook(customHook){};
  void call(V &value) { _customHook(value); };

 private:
  CustomHook _customHook;
};
}  // namespace Hook
#endif