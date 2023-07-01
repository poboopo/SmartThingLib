#ifndef ACTIONS_LIST_H
#define ACTIONS_LIST_H

#include <functional>
#include <ArduinoJson.h>

namespace Configurable {
    namespace Action {
        struct ActionResult {
            ActionResult(){};
            ActionResult(bool successful):successful(successful){};
            ActionResult(bool successful, const char * message):successful(successful), message(message){};
            bool successful = false;
            const char * message = nullptr;
        };

        typedef std::function<ActionResult(void)> ActionHandler;

        struct Action {
            const char * name;
            const char * caption;
            ActionHandler handler;

            Action * next;
            Action * previous;
        };

        class ActionsList {
            public:
                ActionsList(): _head(nullptr){};
                ~ActionsList();

                bool add(const char * actionName, const char * caption, ActionHandler handler);
                ActionResult callAction(const char * actionName);
                DynamicJsonDocument getDict();

                int size() {
                    return _count;
                }
            private:
                Action * _head;
                int _count = 0;
                void append(Action * sensor);
                const Action * findAction(const char * actionName) const;
        };
    }
}

#endif