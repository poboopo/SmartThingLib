#ifndef LAMBDA_HOOK_H
#define LAMBDA_HOOK_H

#include <functional>

#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"

#define LAMBDA_HOOK_TAG "lambda_hook"

namespace Hook {
template <class T>
class LambdaHook : public Hook<T> {
 public:
  typedef std::function<void(T &value)> CustomHook;

  LambdaHook(CustomHook customHook, bool readOnly)
      : Hook<T>(LAMBDA_HOOK_TAG, readOnly),
        _customHook(customHook){};
  void call(T &value) { _customHook(value); };
  DynamicJsonDocument toJson(bool shortJson) {
    DynamicJsonDocument doc(HOOK_INFO_DOC_SIZE);
    this->addDefaultInfo(doc);
    return doc;
  };
  void updateCustom(JsonObject doc){};

 private:
  CustomHook _customHook;
};
}  // namespace Hook
#endif