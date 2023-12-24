#ifndef NOTIFICATION_CALLBACK_H
#define NOTIFICATIONS_CALLBACK_H

#include "callbacks/impls/Callback.h"

namespace Callback {
template <typename T>
class NotificationCallback : public Callback<T> {};
}  // namespace Callback

#endif