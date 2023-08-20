- CURRENT
    - save state
    - add statics rest

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
        - add rest to create callback +
        - put multiple callback for one object in one watcher +
        - execute http callbacks async +
        - add callback scenarios in web page +
        
    - v0.3
        - add settings compression LZ78 - [CANCELLED BCZ USLESS]
        - add different http callback types support +
        - add callabcks temaplate rest for callbacks creation (required fields) +
        - save callbacks to settings +
        - save state +
        - add statics rest
        - fix logger message length
        - replace value in url in HTTP_CALLBACK

    - v0.4
        - add ActionCallback - call action from callback (+ web)
        - todo SmartThing loop task routine - move to async
        - all handlers uri rework
        - add import export config
        - add possible values to device state
        - refactor handlers - make base handler with all routine(log, headers)
        - add restart handler
        - remove white spaces in mobile version of web page

- backlog
    - ОПИШИ АРХИТЕКТУРУ
    - wifi network scan
    - add authorization???
    - tuya, zigbee integration
