- v1.0
    - v0.1
        - make smart thing static +
        - refactor static classes to extern +
        - add logging levels +
        - settings manager refactor +
        - fix mobile web page view +

    - v0.2
        - fix directories +
        - add sensors, state, config setters +
        - change sensors and state response to dict +
        - make sensors and states unique by name (as actions and config) +
        - fix web page + 
        - add watchers +
        - add rest to create hook +
        - put multiple hook for one object in one watcher +
        - execute http hooks async +
        - add hook scenarios in web page +
        
    - v0.3
        - add settings compression LZ78 - [CANCELLED BCZ USLESS]
        - add different http hook types support +
        - add callabcks temaplate rest for hooks creation (required fields) +
        - save hooks to settings +
        - save state +
        - add statics rest +
        - replace value in url in HTTP_HOOK +

    - v0.4
        - add ActionHook - call action from hook (+ web) +
        - SmartThing loop task routine - move to async +
        - add restart handler +
        - all handlers uri rework +
        - rework log message format - just use : or $ to separate data +

- backlog
    - tuya, zigbee integration
    - store sensors values -
    - add tests +
    - add multiple values support for trigger value
    - add message broker support in logger -
    - add different templates support on logger? -
    - fix logger message length

- DynamicJsonDocument -> JsonDocument