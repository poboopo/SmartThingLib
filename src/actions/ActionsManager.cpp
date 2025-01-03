#include "actions/ActionsManager.h"

#if ENABLE_ACTIONS

#include <ArduinoJson.h>

const char * const _ACTIONS_TAG = "actions_manager";

#if ENABLE_ACTIONS_SCHEDULER
  const char * const _actionJsonTemplate = "{\"name\":\"%s\",\"caption\":\"%s\",\"callDelay\":\"%lu\",\"lastCall\":\"%lu\"}%s";
  const size_t _actionJsonTemplateSize = 54;
#else
  const char * const _actionJsonTemplate = "{\"name\":\"%s\",\"caption\":\"%s\"}%s";
  const size_t _actionJsonTemplateSize = 25;
#endif

ActionsManagerClass ActionsManager;

size_t ActionsManagerClass::count() {
  return _actions.size();
}

bool ActionsManagerClass::add(const char* name, const char* caption, ActionHandler handler) {
  if (findAction(name) != nullptr) {
    st_log_warning(_ACTIONS_TAG,
                    "Handler for action %s already exists! Skipping...",
                    name);
    return false;
  }

  Action* action = new Action(name, caption, handler);
  if (_actions.append(action) > -1) {
    st_log_debug(_ACTIONS_TAG, "Added new action handler - %s:%s",
                  name, caption);
    return true;
  } else {
    if (action != nullptr) {
      delete action;
    }
    st_log_error(_ACTIONS_TAG, "Failed to add new action handler - %s:%s", name, caption);
    return false;
  }
};

bool ActionsManagerClass::remove(const char* name) {
  Action * action = findAction(name);
  if (action == nullptr) {
    st_log_warning(_ACTIONS_TAG, "There is no action with name %s", name);
    return false;
  }

  if (!_actions.remove(action)) {
    st_log_error(_ACTIONS_TAG, "Failed to remove action from list");
    return false;
  }
  
  delete action;
  action == nullptr;
  st_log_warning(_ACTIONS_TAG, "Action deleted");
  return true;
}

ActionResult ActionsManagerClass::call(const char* name) {
  const Action* action = findAction(name);
  if (action == nullptr) {
    st_log_error(_ACTIONS_TAG, "Can't find action with name %s", name);
    return ActionResult(false, "Failed to find action");
  }
  return action->call();
};

const Action * ActionsManagerClass::get(const char* name) const {
  return findAction(name);
}

#if ENABLE_ACTIONS_SCHEDULER
void ActionsManagerClass::loadFromSettings() {
  JsonDocument config = SettingsRepository.getActions();
  if (config.size() == 0) {
    st_log_debug(_ACTIONS_TAG, "Actions config empty");
    return;
  }

  _actions.forEach([&](Action * action) {
    if (config[action->name()].is<const char*>()) {
      unsigned long callDelay = config[action->name()];
      action->setCallDelay(callDelay);
    }
  });
}

bool ActionsManagerClass::updateActionSchedule(const char * name, unsigned long newDelay) {
  st_log_debug(_ACTIONS_TAG, "Trying to update action %s delay", name);
  Action* action = findAction(name);
  if (action == nullptr) {
    st_log_error(_ACTIONS_TAG, "Can't find action with name %s", name);
    return false;
  } else {
    JsonDocument config = SettingsRepository.getActions();
    if (newDelay == 0) {
      config.remove(action->name());
    } else {
      config[action->name()] = newDelay;
    }
    SettingsRepository.setActions(config);

    action->setCallDelay(newDelay);
    st_log_info(_ACTIONS_TAG, "Action %s delay was update to %lu", name, newDelay);
    return true;
  }
}

void ActionsManagerClass::scheduled() {
  unsigned long current = millis();
  _actions.forEach([&](Action * action) {
    if (action->callDelay() > 0 && current - action->lastCall() > action->callDelay()) {
      st_log_debug(_ACTIONS_TAG, "Scheduled call of %s", action->name());
      action->call();
      action->setLastCall(current);
    }
  });
}
#endif

void ActionsManagerClass::forEachAction(List<Action>::ForEachIndexFunction forFunc) {
  _actions.forEach(forFunc);
}

String ActionsManagerClass::toJson() {
  unsigned long currentMillis = millis();
  String result = "[";
  int lastIndex = _actions.size() - 1;
  _actions.forEach([&](Action* current, int index) {
    size_t size = _actionJsonTemplateSize +
      strlen(current->name()) + 
      strlen(current->caption()) + 1;

    #if ENABLE_ACTIONS_SCHEDULER
      unsigned long lastCall = current->callDelay() > 0 ? currentMillis - current->lastCall() : 0;
      size += snprintf(NULL, 0, "%d%d", current->callDelay(), lastCall);
    #endif

    char buff[size];
    
    #if ENABLE_ACTIONS_SCHEDULER
      sprintf(
        buff,
        _actionJsonTemplate,
        current->name(),
        current->caption(),
        current->callDelay(),
        lastCall,
        index == lastIndex ? "" : ","
      );
    #else
      sprintf(
        buff,
        _actionJsonTemplate,
        current->name(),
        current->caption(),
        index == lastIndex ? "" : ","
      );
    #endif
    result += String(buff);
  });

  result += "]";

  return result;
};

Action* ActionsManagerClass::findAction(const char* name) const {
  return _actions.findValue([&](Action* current) { return strcmp(current->name(), name) == 0; });
}

#endif