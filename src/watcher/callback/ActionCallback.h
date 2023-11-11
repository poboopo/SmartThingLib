#ifndef ACTION_CALLBACK_H
#define ACTION_CALLBACK_H

#include "watcher/callback/WatcherCallback.h"
#include "logs/BetterLogger.h"
#include "SmartThing.h"

#define ACTION_CALLBACK_TAG "action_callback"

namespace Callback {
    template<class T>
    class ActionCallback: public WatcherCallback<T> {
        public:
            ActionCallback(const char * action, T triggerValue, bool readOnly):
            WatcherCallback<T>(ACTION_CALLBACK_TAG, triggerValue, readOnly), _action(action) {};

            void call(T &value) {
                // replace ${value} in _action?
                LOGGER.debug(ACTION_CALLBACK_TAG, "Calling action  %s", _action.c_str());
                SmartThing.callAction(_action.c_str());
            }

            DynamicJsonDocument toJson(bool shortJson) {
                DynamicJsonDocument doc(CALLBACK_INFO_DOC_SIZE);
                doc["action"] = _action.c_str();
                this->addDefaultInfo(doc);
                return doc;
            }

            void updateCustom(JsonObject obj) {
                if (obj.containsKey("action")) {
                    String newAction = obj["action"].as<String>();
                    if (newAction.isEmpty()) {
                        LOGGER.error(ACTION_CALLBACK_TAG, "Action is missing!");
                        return;
                    }
                    _action = newAction;
                    LOGGER.debug(ACTION_CALLBACK_TAG, "New callback action: %s", _action.c_str());
                }
            } 
            
            static DynamicJsonDocument getTemplate() {
                DynamicJsonDocument doc(MAX_CALLBACK_TEMPLATE_SIZE);
                JsonObject actionObj = doc.createNestedObject("action");
                actionObj["required"] = true;
                actionObj["values"] = SmartThing.getActionsInfo(); 
                return doc;
            }
        private:
            String _action;
    };
}

#endif
