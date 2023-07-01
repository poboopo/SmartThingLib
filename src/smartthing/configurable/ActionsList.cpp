#include "smartthing/configurable/ActionsList.h"
#include "smartthing/logs/BetterLogger.h"

#define ACTIONS_LIST_TAG "action_handlers_list"

using namespace Configurable::Action;

ActionsList::~ActionsList() {
    Action * current = _head;
    while (current->next != nullptr) {
        current = current->next;
        delete(current->previous);
    }
    delete(current);
}

DynamicJsonDocument ActionsList::getDict() {
    DynamicJsonDocument doc(_count * 64);
    Action * current = _head;
    while (current != nullptr) {
        doc[current->name] = current->caption;
        current = current->next;
    }
    return doc;
}

bool ActionsList::add(const char * actionName, const char * caption, ActionHandler handler) {
    if (findAction(actionName) != nullptr) {
        LOGGER.warning(ACTIONS_LIST_TAG, "Handler for action %s already exists! Skipping...", actionName);
        return false;
    }
    Action * action = new Action;
    action->name = actionName;
    action->handler = handler;
    action->caption = caption;
    append(action);
    LOGGER.debug(ACTIONS_LIST_TAG, "Added new action handler - %s:%s", actionName, caption);
    return true;
}

ActionResult ActionsList::callAction(const char * actionName) {
    const Action * action = findAction(actionName);
    if (action == nullptr) {
        LOGGER.warning(ACTIONS_LIST_TAG, "Can't find action with name %s", actionName);
        return ActionResult(false, "Failed to find action");
    }
    return action->handler();
}

void ActionsList::append(Action * action) {
    action->next = _head;
    if (_head != nullptr) {
        _head->previous = action;
    }
    _head = action;
    _count++;
}

const Action * ActionsList::findAction(const char * actionName) const {
    Action * current = _head;
    while (current != nullptr) {
        if (strcmp(current->name, actionName) == 0) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}