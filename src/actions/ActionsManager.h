#ifndef ACTIONS_LIST_H
#define ACTIONS_LIST_H

#include "Features.h"
#if ENABLE_ACTIONS

#include <ArduinoJson.h>
#include <functional>

#include "utils/List.h"
#include "logs/BetterLogger.h"
#include "settings/SettingsRepository.h"

static const char * ACTIONS_JSON_NAME = "name";
static const char * ACTIONS_JSON_CAPTION = "caption";
#if ENABLE_ACTIONS_SCHEDULER
static const char * ACTIONS_JSON_LAST_CALL = "lastCall";
static const char * ACTIONS_JSON_DELAY = "callDelay";
#endif

struct ActionResult {
  ActionResult(){};
  ActionResult(bool successful) : successful(successful){};
  ActionResult(bool successful, const char* message)
      : successful(successful), message(message){};
  bool successful = false;
  const char* message = nullptr;
};

typedef std::function<ActionResult(void)> ActionHandler;

struct Action {
  Action(const char* n, const char* c, ActionHandler h)
  #if ENABLE_ACTIONS_SCHEDULER
    : name(n), caption(c), handler(h), callDelay(0), lastCall(0) {};
  #else
    : name(n), caption(c), handler(h) {};
  #endif
  const char* name;
  const char* caption;
  ActionHandler handler;

  #if ENABLE_ACTIONS_SCHEDULER
  unsigned long callDelay;
  unsigned long lastCall;
  #endif
};

class ActionsManagerClass : protected List<Action> {
 public:
  size_t count();

  bool add(const char* actionName, const char* caption, ActionHandler handler);

  ActionResult call(const char* name) ;

  #if ENABLE_ACTIONS_SCHEDULER
  void loadFromSettings();

  bool updateActionSchedule(const char * name, unsigned long newDelay);

  void scheduled();
  #endif

  JsonDocument toJson();

 private:
  Action* findAction(const char* name);
};

extern ActionsManagerClass ActionsManager;

#endif
#endif
