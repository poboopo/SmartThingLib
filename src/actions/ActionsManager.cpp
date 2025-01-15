#include "actions/ActionsManager.h"

#if ENABLE_ACTIONS

#include <ArduinoJson.h>

#include "logs/BetterLogger.h"
#include "settings/SettingsRepository.h"

const char * const _ACTIONS_TAG = "actions_manager";

#if ENABLE_ACTIONS_SCHEDULER
  const char * const _actionJsonTemplate = "{\"name\":\"%s\",\"caption\":\"%s\",\"callDelay\":%lu,\"lastCall\":%lu}%s";
  const size_t _actionJsonTemplateSize = 50;
#else
  const char * const _actionJsonTemplate = "{\"name\":\"%s\",\"caption\":\"%s\"}%s";
  const size_t _actionJsonTemplateSize = 25;
#endif

ActionsManagerClass ActionsManager;

size_t ActionsManagerClass::count() {
  return _actions.size();
}

bool ActionsManagerClass::add(const char* name, const char* caption, ActionHandler handler) {
  if (findAction(name) != _actions.end()) {
    st_log_warning(_ACTIONS_TAG,
                    "Action with name %s already exists!:",
                    name);
    return false;
  }

  Action * action = new Action(name, caption, handler);
  _actions.push_back(action);
  st_log_debug(_ACTIONS_TAG, "Added new action handler - %s:%s", action->name(), action->caption());
  return true;
};

bool ActionsManagerClass::remove(const char* name) {
  auto action = findAction(name);
  if (action == _actions.end()) {
    st_log_warning(_ACTIONS_TAG, "There is no action with name %s", name);
    return false;
  }

  delete *action;
  *action = nullptr;
  _actions.erase(action);

  st_log_warning(_ACTIONS_TAG, "Action %s removed", name);
  return true;
}

ActionResultCode ActionsManagerClass::call(const char* name) {
  auto action = findAction(name);
  if (action == _actions.end()) {
    st_log_error(_ACTIONS_TAG, "Can't find action with name %s", name);
    return ACTION_RESULT_NOT_FOUND;
  }
  st_log_info(_ACTIONS_TAG, "Calling action name=%s", name);
  return (*action)->call() ? ACTION_RESULT_SUCCESS : ACTION_RESULT_ERROR;
};

#if ENABLE_ACTIONS_SCHEDULER
void ActionsManagerClass::loadFromSettings() {
  JsonDocument config = SettingsRepository.getActions();
  if (config.size() == 0) {
    st_log_debug(_ACTIONS_TAG, "Actions config empty");
    return;
  }

  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    Action * action = *it;
    if (config[action->name()].is<const char*>()) {
      unsigned long callDelay = config[action->name()];
      action->setCallDelay(callDelay);
    }
  }
}

bool ActionsManagerClass::updateActionSchedule(const char * name, unsigned long newDelay) {
  st_log_debug(_ACTIONS_TAG, "Trying to update action %s delay", name);
  auto it = findAction(name);
  if (it == _actions.end()) {
    st_log_error(_ACTIONS_TAG, "Can't find action with name %s", name);
    return false;
  } else {
    Action *action = *it;
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

  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    Action * action = *it;
    if (action->callDelay() > 0 && current - action->lastCall() > action->callDelay()) {
      st_log_debug(_ACTIONS_TAG, "Scheduled call of %s", action->name());
      action->call();
      action->setLastCall(current);
    }
  }
}
#endif

String ActionsManagerClass::getActionsInfoForHook() {
  String result;
  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    Action * action = *it;

    char data[strlen(action->name()) + strlen(action->caption()) + 6];
    sprintf(
      data,
      "\"%s\":\"%s\"%s",
      action->name(),
      action->caption(),
      it == std::prev(_actions.end()) ? "" : ","
    );
    result += String(data);
  }
  return result;
}

String ActionsManagerClass::toJson() {
  unsigned long currentMillis = millis();
  String result = "[";
  int lastIndex = _actions.size() - 1;


  for (auto it = _actions.begin(); it != _actions.end(); ++it) {
    Action * action = *it;

    size_t size = _actionJsonTemplateSize +
      strlen(action->name()) + 
      strlen(action->caption()) + 1;

    #if ENABLE_ACTIONS_SCHEDULER
      unsigned long lastCall = action->callDelay() > 0 ? currentMillis - action->lastCall() : 0;
      size += snprintf(NULL, 0, "%d%d", action->callDelay(), lastCall);
    #endif

    char buff[size];
    bool isLast = it == std::prev(_actions.end());
    
    #if ENABLE_ACTIONS_SCHEDULER
      sprintf(
        buff,
        _actionJsonTemplate,
        action->name(),
        action->caption(),
        action->callDelay(),
        lastCall,
        isLast ? "" : ","
      );
    #else
      sprintf(
        buff,
        _actionJsonTemplate,
        action->name(),
        action->caption(),
        isLast ? "" : ","
      );
    #endif
    result += String(buff);
  }

  result += "]";

  return result;
};

std::list<Action*>::iterator ActionsManagerClass::findAction(const char* name) {
  return std::find_if(_actions.begin(), _actions.end(), [name](const Action * action) {
    return strcmp(action->name(), name) == 0;
  });
}

#endif