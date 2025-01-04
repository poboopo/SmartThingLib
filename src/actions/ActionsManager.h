#ifndef ACTIONS_LIST_H
#define ACTIONS_LIST_H

#include "Features.h"
#if ENABLE_ACTIONS

#include <Arduino.h>
#include <functional>
#include <list>

// todo copy message and free on deestroy
struct ActionResult {
  ActionResult(){};
  ActionResult(bool successful) : successful(successful){};
  ActionResult(bool successful, const char* message)
      : successful(successful), message(message){};
  bool successful = false;
  const char* message = nullptr;
};

typedef std::function<ActionResult(void)> ActionHandler;

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

    ActionResult call() const {
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

class ActionsManagerClass {
 public:
  size_t count();

  bool add(const char* name, const char* caption, ActionHandler handler);
  bool remove(const char* name);

  ActionResult call(const char* name);

  String getActionsInfoForHook();

  #if ENABLE_ACTIONS_SCHEDULER
  void loadFromSettings();

  bool updateActionSchedule(const char * name, unsigned long newDelay);

  void scheduled();
  #endif

  String toJson();

 private:
  std::list<Action*> _actions;

  std::list<Action*>::iterator findAction(const char* name);
};

extern ActionsManagerClass ActionsManager;

#endif
#endif
