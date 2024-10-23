#include "actions/ActionsManager.h"

#if ENABLE_ACTIONS

static const char * ACTIONS_TAG = "actions_manager";

ActionsManagerClass ActionsManager;

size_t ActionsManagerClass::count() {
  return size();
}

bool ActionsManagerClass::add(const char* actionName, const char* caption, ActionHandler handler) {
  if (findAction(actionName) != nullptr) {
    SMT_LOG_WARNING(ACTIONS_TAG,
                    "Handler for action %s already exists! Skipping...",
                    actionName);
    return false;
  }
  Action* action = new Action(actionName, caption, handler);
  if (append(action) > -1) {
    SMT_LOG_DEBUG(ACTIONS_TAG, "Added new action handler - %s:%s",
                  actionName, caption);
    return true;
  } else {
    if (action != nullptr) {
      delete action;
    }
    SMT_LOG_ERROR(ACTIONS_TAG, "Failed to add new action handler - %s:%s", actionName, caption);
    return false;
  }
};

// todo rename to call
ActionResult ActionsManagerClass::call(const char* name) {
  SMT_LOG_DEBUG(ACTIONS_TAG, "Trying to call action %s", name);
  const Action* action = findAction(name);
  if (action == nullptr) {
    SMT_LOG_ERROR(ACTIONS_TAG, "Can't find action with name %s", name);
    return ActionResult(false, "Failed to find action");
  }
  return action->handler();
};

#if ENABLE_ACTIONS_SCHEDULER
void ActionsManagerClass::loadFromSettings() {
  JsonDocument config = STSettings.getActionsConfig();
  if (config.size() == 0) {
    SMT_LOG_DEBUG(ACTIONS_TAG, "Actions config empty");
    return;
  }

  forEach([&](Action * action) {
    if (config.containsKey(action->name)) {
      unsigned long callDelay = config[action->name];
      action->callDelay = callDelay;
    }
  });
}

bool ActionsManagerClass::updateActionDelay(const char * name, unsigned long newDelay) {
  SMT_LOG_DEBUG(ACTIONS_TAG, "Trying to update action %s delay", name);
  Action* action = findAction(name);
  if (action == nullptr) {
    SMT_LOG_ERROR(ACTIONS_TAG, "Can't find action with name %s", name);
    return false;
  } else {
    JsonDocument config = STSettings.getActionsConfig();
    if (newDelay == 0) {
      config.remove(action->name);
    } else {
      config[action->name] = newDelay;
    }
    STSettings.setActionsConfig(config);
    STSettings.save();

    action->callDelay = newDelay;
    SMT_LOG_INFO(ACTIONS_TAG, "Action %s delay was update to %lu", name, newDelay);
    return true;
  }
}

void ActionsManagerClass::scheduled() {
  long current = millis();
  forEach([&](Action * action) {
    if (action->callDelay > 0 && current - action->lastCall > action->callDelay) {
      SMT_LOG_DEBUG(ACTIONS_TAG, "Scheduled call of %s", action->name);
      action->handler();
      action->lastCall = current;
    }
  });
}
#endif

JsonDocument ActionsManagerClass::toJson() {
  JsonDocument doc;
  doc.to<JsonArray>();
  long currentMillis = millis();
  forEach([&](Action* current) {
    JsonDocument action;
    action[ACTIONS_JSON_NAME] = current->name;
    action[ACTIONS_JSON_CAPTION] = current->caption;

    #if ENABLE_ACTIONS_SCHEDULER
    action[ACTIONS_JSON_DELAY] = current->callDelay;
    action[ACTIONS_JSON_LAST_CALL] = current->callDelay > 0 ? currentMillis - current->lastCall : 0;
    #endif

    doc.add(action);
  });
  return doc;
};

Action* ActionsManagerClass::findAction(const char* name) {
  return findValue([&](Action* current) { return strcmp(current->name, name) == 0; });
}

#endif