#ifndef ACTION_HOOK_H
#define ACTION_HOOK_H

#include "Features.h"

#if ENABLE_ACTIONS 

#include <type_traits>
#include "SmartThing.h"
#include "hooks/impls/Hook.h"
#include "logs/BetterLogger.h"

static const char * ACTION_HOOK_TAG = "action_hook";

template<class T, typename V, typename std::enable_if<std::is_base_of<Hook<V>, T>::value>::type* = nullptr>
class ActionHook : public T {
  public:
    ActionHook(const char *action, bool readOnly): T(ACTION_HOOK_TAG, readOnly) {
      _action = (char *) malloc(strlen(action) + 1);
      strcpy(_action, action);
    };
    virtual ~ActionHook() {};

    void call(V &value) {
      // replace ${value} in _action?
      st_log_debug(ACTION_HOOK_TAG, "Calling action  %s", _action);
      ActionsManager.call(_action);
    }

    void populateJsonWithCustomValues(JsonDocument &doc, boolean shortJson) const {
      doc["action"] = _action;
    };

    void updateCustom(JsonObject obj) {
      if (obj.containsKey("action")) {
        String newAction = obj["action"].as<String>();
        if (newAction.isEmpty()) {
          st_log_error(ACTION_HOOK_TAG, "Action is missing!");
          return;
        }
        free(_action);
        _action = (char *) malloc(newAction.length() + 1);
        strcpy(_action, newAction.c_str());
        st_log_debug(ACTION_HOOK_TAG, "New hook action: %s",  _action);
      }
    }

  private:
    char * _action;
};

#endif

#endif
