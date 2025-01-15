#ifndef ACTIONS_LIST_H
#define ACTIONS_LIST_H

#include "Features.h"
#if ENABLE_ACTIONS

#include <Arduino.h>
#include <functional>
#include <list>

enum ActionResultCode {
  ACTION_RESULT_NOT_FOUND = -1,
  ACTION_RESULT_ERROR = 0,
  ACTION_RESULT_SUCCESS = 1,
};

typedef std::function<bool(void)> ActionHandler;

class Action {
  public:
    Action(const char* name, const char* caption, ActionHandler h)
    #if ENABLE_ACTIONS_SCHEDULER
      : _handler(h), _callDelay(0), _lastCall(0)
    #else
      : _handler(h)
    #endif
    {
      _name = (char *) malloc(strlen(name) + 1);
      strcpy(_name, name);

      if (strcmp(name, caption) == 0) {
        _caption = _name;
      } else {
        _caption = (char *) malloc(strlen(caption) + 1);
        strcpy(_caption, caption);
      }
    };

    ~Action() {
      if (_name != _caption) {
        free(_caption);
      }
      free(_name);
    };

    const char * name() const {
      return _name;
    }

    const char * caption() const {
      return _caption;
    }

    bool call() const {
      return _handler();
    }

    #if ENABLE_ACTIONS_SCHEDULER
      unsigned long callDelay() const {
        return _callDelay;
      }

      unsigned long lastCall() const {
        return _lastCall;
      }

      void setCallDelay(unsigned long callDelay) {
        _callDelay = callDelay;
      }

      void setLastCall(unsigned long lastCall) {
        _lastCall = lastCall;
      }
    #endif

  private:
    char* _name;
    char* _caption;
    ActionHandler _handler;

    #if ENABLE_ACTIONS_SCHEDULER
    unsigned long _callDelay;
    unsigned long _lastCall;
    #endif
};

// Manager for device actions configuration
class ActionsManagerClass {
 public:
  /*
    Add action
    @param name system action name. This name will be used to call action over api.
    @param caption action caption for UI.
    @param handler lambda with action's logic.
    @returns true if action added.
  */
  bool add(const char* name, const char* caption, ActionHandler handler);

  /*
    Remove action
    @param name system action name.
    @returns true if action removed.
  */
  bool remove(const char* name);

  /*
    Call previously added action.
    @param name system action name.
    @returns result code ActionResultCode (1 - success, -1 or 0 - failure)
  */
  ActionResultCode call(const char* name);

  #if ENABLE_ACTIONS_SCHEDULER
    void loadFromSettings();
    bool updateActionSchedule(const char * name, unsigned long newDelay);
    void scheduled();
  #endif

  String getActionsInfoForHook();
  String toJson();
  size_t count();
 private:
  std::list<Action*> _actions;

  std::list<Action*>::iterator findAction(const char* name);
};

extern ActionsManagerClass ActionsManager;

#endif
#endif
