#ifndef ACTIONS_LIST_H
#define ACTIONS_LIST_H

#include "Features.h"
#if ENABLE_ACTIONS 

#include <ArduinoJson.h>

#include <functional>

#include "utils/List.h"
#include "logs/BetterLogger.h"

#define ACTIONS_LIST_TAG "action_handlers_list"

struct ActionResult {
  ActionResult(){};
  ActionResult(bool successful) : successful(successful){};
  ActionResult(bool successful, const char* message)
      : successful(successful), message(message){};
  bool successful = false;
  const char* message = nullptr;
};

namespace Action {
typedef std::function<ActionResult(void)> ActionHandler;

struct Action {
  Action(const char* n, const char* c, ActionHandler h)
      : name(n), caption(c), handler(h){};
  const char* name;
  const char* caption;
  ActionHandler handler;
};

class ActionsList : public List<Action> {
 public:
  bool add(const char* actionName, const char* caption, ActionHandler handler) {
    if (findAction(actionName) != nullptr) {
      LOGGER.warning(ACTIONS_LIST_TAG,
                     "Handler for action %s already exists! Skipping...",
                     actionName);
      return false;
    }
    Action* action = new Action(actionName, caption, handler);
    if (append(action) > -1) {
      LOGGER.debug(ACTIONS_LIST_TAG, "Added new action handler - %s:%s",
                   actionName, caption);
      return true;
    } else {
      if (action != nullptr) {
        delete action;
      }
      LOGGER.error(ACTIONS_LIST_TAG, "Failed to add new action handler - %s:%s",
                   actionName, caption);
      return false;
    }
  };

  ActionResult callAction(const char* actionName) {
    const Action* action = findAction(actionName);
    if (action == nullptr) {
      LOGGER.error(ACTIONS_LIST_TAG, "Can't find action with name %s",
                   actionName);
      return ActionResult(false, "Failed to find action");
    }
    return action->handler();
  };

  DynamicJsonDocument toJson() {
    DynamicJsonDocument doc(size() * 64);
    forEach([&](Action* current) { doc[current->name] = current->caption; });
    return doc;
  };

 private:
  const Action* findAction(const char* name) {
    return findValue(
        [&](Action* current) { return strcmp(current->name, name) == 0; });
  }
};
}  // namespace Action
#endif

#endif