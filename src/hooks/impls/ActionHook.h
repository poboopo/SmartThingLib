#ifndef ACTION_HOOK_H
#define ACTION_HOOK_H

#include <type_traits>
#include "SmartThing.h"
#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"

#define ACTION_HOOK_TAG "action_hook"

namespace Hook {
  template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
  class ActionHook : public T {
    public:
      ActionHook(const char *action, bool readOnly)
          : T(ACTION_HOOK_TAG, readOnly), _action(action){};

      void call(V &value) {
        // replace ${value} in _action?
        LOGGER.debug(ACTION_HOOK_TAG, "Calling action  %s", _action.c_str());
        SmartThing.callAction(_action.c_str());
      }

      void addCustomJsonValues(DynamicJsonDocument doc, boolean shortJson) {
        doc["action"] = _action.c_str();
      };

      void updateCustom(JsonObject obj) {
        if (obj.containsKey("action")) {
          String newAction = obj["action"].as<String>();
          if (newAction.isEmpty()) {
            LOGGER.error(ACTION_HOOK_TAG, "Action is missing!");
            return;
          }
          _action = newAction;
          LOGGER.debug(ACTION_HOOK_TAG, "New hook action: %s",
                      _action.c_str());
        }
      }

    private:
      String _action;
  };
}  // namespace Hook

#endif
