#ifndef LAMBDA_CALLBACK_H
#define LAMBDA_CALLBACK_H

#include <functional>

#include "callbacks/impls/Callback.h"
#include "logs/BetterLogger.h"

#define LAMBDA_CALLBACK_TAG "lambda_callback"

namespace Callback {
template <class T>
class LambdaCallback : public Callback<T> {
 public:
  typedef std::function<void(T &value)> CustomCallback;

  LambdaCallback(CustomCallback customCallback, bool readOnly)
      : Callback<T>(LAMBDA_CALLBACK_TAG, readOnly),
        _customCallback(customCallback){};
  void call(T &value) { _customCallback(value); };
  DynamicJsonDocument toJson(bool shortJson) {
    DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE);
    this->addDefaultInfo(doc);
    return doc;
  };
  void updateCustom(JsonObject doc){};

 private:
  CustomCallback _customCallback;
};
}  // namespace Callback
#endif