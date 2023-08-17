const String WEB_PAGE_MAIN = R"=====(
<html>
    <head>
        <title>SmartThing control page</title>
    </head>
    <body>
        <div class="main-panel">
            <div id="info" class="content-block">
                <div class="loading-info"><p>Loading</p></div>
                <h1>Device info</h1>
                <div id="device-info" class="grid-view"></div>
                <button title="Restart device" class="btn-restart" onclick="restart()">Restart</button>
            </div>
            <div id="wifi" class="content-block">
                <div class="loading-info">Loading</div>
                <h1>WiFi settings</h1>
                <div class="grid-view">
                    <p>WiFi network name</p>
                    <input type="text" id="ssid" title="SSID"/>
                    <p>WiFi password</p>
                    <input type="password" id="password" title="password">
                    <p>WiFi mode</p>
                    <select id="wifi-mode" title="mode"></select>
                </div>
                <div class="btn-group">
                    <button onclick="saveWifiSettings()">Save and reconnect</button>
                </div>
            </div>
            <div id="actions"class="content-block btn-group hidable">
                <div class="loading-info">Loading</div>
                <h1>Actions</h1>
                <div id="control-buttons-block"></div>
            </div>
            <div id="config" class="content-block hidable">
                <div class="loading-info">Loading</div>
                <h1>Config</h1>
                <div id="config-fields-block" class="grid-view"></div>
                <div class="btn-group" >
                    <button title="Save config values" onclick="saveConfig()">save</button>
                    <button title="Save config values" style="background-color: red;" onclick="deleteConfig()">delete all</button>
                </div>
                <button class="update-button" onclick="loadConfigValues()">Update</button>
            </div>
            <div id="state" class="content-block hidable">
                <div class="loading-info">Loading</div>
                <h1>State</h1>
                <button class="update-button" onclick="loadState()">Update</button>
                <div id="state-fields-block" class="grid-view"></div>
            </div>
            <div id="sensors" class="content-block hidable">
                <div class="loading-info">Loading</div>
                <h1>Sensors values</h1>
                <button class="update-button" onclick="loadSensors()">Update</button>
                <div id="sensors-fields-block" class="grid-view"></div>
            </div>
            <div id="callbacks" class="content-block hidable">
                <div class="loading-info">Loading</div>
                <h1 id="callbacks-title"></h1>
                <div class="btn-group">
                    <button id="add-new-callback">Add new</button>
                </div>
                <button class="update-button" onclick="updateCallbacks()">Update</button>
                <div id="callbacks-block" style="text-align: start;"></div>
            </div>
        </div>
    </body>
    <script>
        this.config = {};
        this.dictionaries = {};

        window.onload = function() {
            loadDeviceInfo();
            loadWiFiSettings();
            loadDictionaries();
            loadState();
            loadSensors();
        };
        function updateCallbacks() {
            const callbacksBlock = document.getElementById("callbacks-block");
            if(callbacksBlock) {
                loadCallbacks(callbacksBlock.getAttribute("observableType"), callbacksBlock.getAttribute("observable"));
            }
        }
        function loadCallbacks(observableType, observable) {
            if (!observableType || !observable) {
                console.warn("Callback observableType or observable are missing!");
                return;
            }
            restRequest(
                "GET",
                "http://" + getHost() + "/callbacks/by/observable?observableType=" + observableType + "&name=" + observable,
                null,
                function (response) {
                    if (response) {
                        displayCallbacks(observableType, observable, JSON.parse(response));
                    }
                },
                "callbacks"
            );
        }

        function addCallback(observableType, observable) {
            if (document.getElementById("callback_new")) {
                return;
            }
            const callbacksBlock = document.getElementById("callbacks-block");
            if (!callbacksBlock) {
                console.error("Can't find callbacks-block!");
                return;
            }
            const template = this.callbackTemplates["http_callback"] || {};
            const additionalFields = Object.entries(template).reduce((acc, [key, _]) => {acc[key] = ""; return acc;}, {});
            callbacksBlock.prepend(
                buildCallbackView(observableType, observable, {
                    "type": "http_callback",
                    "readonly": false,
                    "caption": "New http callback",
                    "trigger": "",
                    ...additionalFields
                }, "new")
            );
        }
        function displayCallbacks(observableType, observable, callbacks) {
            const callbacksBlock = document.getElementById("callbacks-block");
            if (!callbacksBlock) {
                console.error("Can't find callbacks-block!");
                return;
            }
            callbacksBlock.setAttribute("observableType", observableType);
            callbacksBlock.setAttribute("observable", observable);
            callbacksBlock.innerHTML = "";
            document.getElementById("callbacks").style.display = "block";

            if (callbacks) {
                callbacks.forEach((callback, index) => {
                    callbacksBlock.appendChild(buildCallbackView(observableType, observable, callback, index));
                });
            }   else {
                console.warn("Callbacks list are empty!");
            }

            const addButton = document.getElementById("add-new-callback");
            if (addButton) {
                addButton.onclick = () => {
                    addCallback(observableType, observable);  
                };
            }
            document.getElementById("callbacks-title").innerHTML = observableType + " " + observable + "'s callbacks";
        }
        function buildCallbackView(observableType, observable, callback, index) {
            const callbackType = callback["type"];
            const div = document.createElement("div");
            div.id = "callback_" + index;
            div.className = "content-block";
            const h2 = document.createElement("h2");
            h2.innerHTML = callback["caption"] || callbackType;
            div.appendChild(h2);
            const block = document.createElement("div");
            block.className = "grid-view";
            const template = this.callbackTemplates[callbackType] || {};
            Object.entries(callback).filter(([key, _]) => key != "type" && key != "caption" && key != "readonly")
                .forEach(([key, value]) => {
                    createEntryInput(
                        block,
                        "callback_" + key + "_" + index,
                        key,
                        value != null ? String(value) : "",
                        key,
                        callback.readonly || (key != "trigger" && !template[key])
                    );
                });
            div.appendChild(block);

            if (!callback.readonly) {
                const wrap = document.createElement("div");
                wrap.classList.add("btn-group", "grid-view");
                const button = document.createElement("button");
                button.innerHTML = "save"
                button.onclick = () => saveCallback(callbackType, observableType, observable, index);
                wrap.appendChild(button);
                if (index != "new") {
                    const deleteButton = document.createElement("button");
                    deleteButton.innerHTML = "delete";
                    deleteButton.style.backgroundColor = "red";
                    deleteButton.onclick = () => {
                        if (confirm("Are you sure you want to delete this callback?")) {
                            deleteCallback(observableType, observable, index);
                        }
                    };
                    wrap.appendChild(deleteButton);
                }
                div.appendChild(wrap);
            }
            return div;
        }
        function saveCallback(callbackType, observableType, observable, index) {
            if (!index && index !== 0) {
                console.error("Index are missing!")
                return;
            }
            const template = this.callbackTemplates[callbackType] || {};
            const reqPayload = {observable: {type: observableType, name: observable}};
            const callbackInfo = {};
            const triggerInput = document.getElementById("callback_trigger_" + index);
            if (triggerInput) {
                callbackInfo["trigger"] = triggerInput.value;
            }

            let valid = true;
            Object.entries(template).forEach(([key, {required}]) => {
                const input = document.getElementById("callback_" + key + "_" + index);
                if (input) {
                    const value = input.value;
                    if (!value && value != 0) {
                        valid += !required;
                    }
                    callbackInfo[key] = value;
                }
            })

            if (Object.keys(callbackInfo).length === 0) {
                console.error("Empty callback payload! Skipping");
                return;
            }

            if (!valid) {
                console.error("Validation failed!");
                return;
            }
            reqPayload["callback"] = callbackInfo;

            if (index == "new") {
                restRequest(
                    "POST",
                    "http://" + getHost() + "/callbacks",
                    reqPayload,
                    (response) => {
                        console.log("Callback created!");
                        loadCallbacks(observableType, observable);
                    },
                    "callbacks"
                )
            } else {
                const queryPart = "?observableType=" + observableType + "&name=" + observable + "&index=" + index;
                restRequest(
                    "PUT",
                    "http://" + getHost() + "/callbacks" + queryPart,
                    reqPayload,
                    (response) => {
                        console.log("Callback updated!");
                    },
                    "callbacks"
                )
            }
        }
        function deleteCallback(observableType, observable, index) {
            const queryPart = "?observableType=" + observableType + "&name=" + observable + "&index=" + index;
            restRequest(
                "DELETE",
                "http://" + getHost() + "/callbacks" + queryPart,
                null,
                (response) => {
                    console.log("Callback deleted :(");
                    loadCallbacks(observableType, observable);
                },
                "callbacks"
            )
        }
        function loadDeviceInfo() {
            restRequest(
                "GET",
                "http://" + getHost() + "/info/system",
                null,
                function (response) {
                    if (response) {
                        this.deviceInfo = JSON.parse(response);
                        processDeviceInfo();
                    }
                },
                "info"
            );
        }
        function processDeviceInfo() {
            const block = document.getElementById("device-info");
            if (this.deviceInfo && block) {
                block.innerHTML = "";
                Object.entries(this.deviceInfo).forEach(([key, value]) => {
                    if (key === "name") {
                        createEntryInput(
                            block,
                            block.id + "-" + key,
                            key,
                            value,
                            "Insert new name",
                            false,
                            "Save",
                            () => saveNewName(),
                            "Save new device name"
                        );
                    } else {
                        createEntryInput(
                            block,
                            block.id + "-" + key,
                            key,
                            value,
                            key,
                            true
                        );
                    }
                });
            }
        }
        function saveNewName() {
            const newName = document.getElementById("device-info-name");
            if (newName && newName.value) {
                restRequest(
                    "PUT",
                    "http://" + getHost() + "/info",
                    {name: newName.value},
                    null,
                    "info"
                );
            }
        }
        function loadWiFiSettings() {
            restRequest("GET", "http://" + getHost() + "/wifi", null,
                function (response) {
                    if (response) {
                        response.trim();
                        const data = JSON.parse(response);
                        if (data["settings"]) {
                            document.getElementById("ssid").value = data["settings"]["ss"] || "";
                            document.getElementById("password").value = data["settings"]["ps"] || "";
                            fillComboBox("wifi-mode", data["modes"], data["settings"]["md"]);
                        }
                    }
                },
                "wifi"
            );
        }
        function fillComboBox(comboboxId, values, selectedValue) {
            if (!values) {
                return;
            }
            const combobox = document.getElementById(comboboxId);
            if (combobox) {
                combobox.innerHTML = "";
                values.forEach((data) => {
                    const option = document.createElement("option");
                    option.innerHTML = data["caption"];
                    option.value = data["value"];
                    combobox.appendChild(option);
                });
                if (selectedValue) {
                    combobox.value = selectedValue;
                }
            }
        }
        function saveWifiSettings() {
            const ssid = document.getElementById("ssid").value;
            if (!ssid || !ssid.length) {
                document.getElementById("ssid").style.backgroundColor = red;
                return;
            }
            document.getElementById("ssid").style.backgroundColor = null;
            const pass = document.getElementById("password").value;
            const mode = document.getElementById("wifi-mode").value;
            restRequest(
                "POST",
                "http://" + getHost() + "/wifi",
                { ssid: ssid, password: pass, mode: mode },
                null,
                "wifi"
            );
        }
        function loadConfigValues() {
            restRequest(
                "GET",
                "http://" + getHost() + "/config",
                null,
                function (response) {
                    if (response) {
                        const loadedConfig = JSON.parse(response);
                        if (loadedConfig) {
                            Object.keys(this.config).forEach((key) => {
                                this.config[key] = loadedConfig[key];
                                document.getElementById(key).value = loadedConfig[key];
                            });
                        }
                    }
                },
                "config"
            );
        }
        function deleteConfig() {
            if (confirm("Are you sure you want to delete ALL config values?")) {
                restRequest(
                    "DELETE",
                    "http://" + getHost() + "/config/remove/all",
                    null,
                    function(response) {
                        loadConfigValues();
                    },
                    "config"
                )
            }
        }
        function saveConfig() {
            if(!this.config) {
                this.config = {};
                return;
            }

            Object.keys(this.config).forEach((key) => {
                const element = document.getElementById(key);
                if (element) {
                    if (element.type === 'number') {
                        this.config[key] = parseInt(element.value);
                    } else {
                        this.config[key] = element.value;
                    }
                } else {
                    console.error("Can't find field value for " + key);
                }
            });

            restRequest(
                "POST",
                "http://" + getHost() + "/config",
                Object.fromEntries(Object.entries(this.config).filter(([_, v]) => v)),
                null,
                "config"
            );
        }
        function deleteConfigValue(name) {
            if (name) {
                restRequest(
                    "DELETE",
                    "http://" + getHost() + "/config/remove?name=" + name,
                    null,
                    function(response) {
                        document.getElementById(name).value = null;
                    },
                    "config"
                )
            }
        }
        function loadState() {
            restRequest(
                "GET",
                "http://" + getHost() + "/state",
                null,
                function (response) {
                    if (response) {
                        response.trim();
                        document.getElementById("state").style.display = "block";
                        const block = document.getElementById("state-fields-block");
                        const states =  JSON.parse(response);
                        if (block && states) {
                            block.innerHTML = "";
                            Object.entries(states).forEach(([key, value]) => {
                                createEntryInput(block, "state_" + key, key, value, key, true,
                                    "Callbacks",
                                    () => loadCallbacks("state", key),
                                    "Device state callbacks"
                                );
                            });
                        }
                    }
                },
                "state"
            );
        }
        function loadSensors() {
            restRequest(
                "GET",
                "http://" + getHost() + "/sensors",
                null,
                function (response) {
                    if (response) {
                        response.trim();
                        document.getElementById("sensors").style.display = "block";
                        const block = document.getElementById("sensors-fields-block");
                        const sensors = JSON.parse(response);
                        if (block && sensors) {
                            block.innerHTML = "";
                            Object.entries(sensors).forEach(([key, {value, type}]) => {
                                createEntryInput(block, "sensor_" + key, key, value, key, true,
                                    "Callbacks",
                                    () => loadCallbacks("sensor", key),
                                    "Sensors callbacks"
                                );
                            });
                        }
                    }
                },
                "sensors"
            );
        }
        function loadDictionaries() {
            restRequest(
                "GET",
                "http://" + getHost() + "/info/actions",
                null,
                function(response) {
                    if (response) {
                        const actions = JSON.parse(response);
                        processActions(actions);
                    }
                }
            )
            restRequest(
                "GET",
                "http://" + getHost() + "/info/config",
                null,
                function(response) {
                    if (response) {
                        const configEntries = JSON.parse(response);
                        processConfigEntries(configEntries);
                        loadConfigValues();
                    }
                }
            )
            restRequest(
                "GET",
                "http://" + getHost() + "/callbacks/template",
                null,
                (response) => {
                    if (response) {
                        this.callbackTemplates = JSON.parse(response);
                    }
                }
            )
        }
        function processActions(actions) {
            if (actions) {
                document.getElementById("actions").style.display = "block";
                const actionsBlock = document.getElementById("control-buttons-block");
                Object.entries(actions).forEach(([action, caption]) => {
                    const button = document.createElement("button");
                    button.onclick = function() {
                        if (action || action == 0) {
                            restRequest("PUT", "http://" + getHost() + "/action?action=" + action, null, null, "actions");
                        } else {
                            console.error("Action is missing!");
                        }
                    };
                    button.innerHTML = caption;
                    actionsBlock.appendChild(button);
                });
            }
        }
        function processConfigEntries(configFields) {
            if (configFields) {
                document.getElementById("config").style.display = "block";
                const configFieldsBlock = document.getElementById("config-fields-block");
                Object.entries(configFields).forEach(([name, {caption, type}]) => {
                    this.config[name] = null;
                    const p = document.createElement("p");
                    p.innerHTML = caption;
                    p.for = name;
                    const input = document.createElement("input");
                    input.type = type;
                    input.id = name;
                    const button = document.createElement("button");
                    button.innerHTML = "X";
                    button.title = "Clear config value";
                    button.style.backgroundColor = "rgb(175, 53, 53)";
                    button.onclick = function () {
                        deleteConfigValue(name);
                    };
                    const div = document.createElement("div");
                    div.className = "config-block";
                    div.appendChild(input);
                    div.appendChild(button);
                    configFieldsBlock.appendChild(p);
                    configFieldsBlock.appendChild(div);
                });
            }
        }
        function restart() {
            restRequest("PUT", "http://" + getHost() + "/restart");
        }
        function displayBlockInfo(blockId, visible=true, text="loading", color="#04AA6D") {
            const block = document.getElementById(blockId);
            if (block) {
                const loadingInfos = block.getElementsByClassName("loading-info");
                if (loadingInfos) {
                    Array.from(loadingInfos).forEach(l => {
                        l.style.display = visible ? "block" : "none";
                        l.innerHTML = text;
                        l.style.backgroundColor = color;
                    });
                }
            }
        }
        function getHost() {
            const { host } = window.location;
            return host;
            // return "192.168.1.103";
        }
        function restRequest(method, path, data, callback, blockId) {
            if (blockId) displayBlockInfo(blockId);
            let xhr = new XMLHttpRequest();
            xhr.open(method, path);
            xhr.setRequestHeader("Accept", "application/json");
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4 ){
                        if (xhr.status >= 200 && xhr.status <= 299) {
                            if (callback) {
                                try {
                                    callback(xhr.response);
                                } catch (exception) {
                                    console.error("Callback failed for " + path, exception);
                                }
                            }
                            if (blockId) displayBlockInfo(blockId, false);
                        } else {
                            console.error("Request " + path + " failed with code " + xhr.status);
                            if (blockId) displayBlockInfo(blockId, true, "Failed", "rgb(175, 53, 53)");
                        }
                    }
                };
            xhr.send(data ? JSON.stringify(data) : null);
        }
        function createEntryInput(container, inputId, caption, value, inputTitle, disabled, buttonCaption, buttonCallback, buttonTitle, backgroundColor = "#04AA6D") {
            const p = document.createElement("p");
            p.innerHTML = caption;
            container.appendChild(p);
            const input = document.createElement("input");
            input.value = value;
            input.id = inputId;
            input.disabled = disabled;
            if (buttonCaption && buttonCallback) {
                input.title = inputTitle;
                const button = document.createElement("button");
                button.style.backgroundColor = backgroundColor;
                button.innerHTML = buttonCaption;
                button.title = buttonTitle;
                button.onclick = buttonCallback;
                const div = document.createElement("div");
                div.className = "config-block";
                div.appendChild(input);
                div.appendChild(button);
                container.appendChild(div);
            } else {
                container.appendChild(input);
            }
        }
    </script>
    <style>
        body {
            background-color: aliceblue;
        }
        * {
            border-radius: 20px;
        }
        p, input, button, select, li, label, .loading-info {
            font-size: 50px;
        }
        h1 {
            font-size: 60px;
        }
        .header {
            font-size: 50px;
        }
        .loading-info {
            position: absolute;
            top: 5px;
            left: 5px;
            padding: 5px;
            background-color: #04AA6D;
            text-align: center;
            display: none;
        }
        .main-panel {
            display: flex;
            flex-wrap: wrap;
            flex-direction: column;
            gap: 1rem;
        }
        @media only screen and (min-width: 1000px) {
            * {
                border-radius: 10px;
            }
            p, input, button, select, li, label, .loading-info {
                font-size: 20px;
            }
            h1 {
                font-size: 30px;
            }
            .main-panel {
                display: flex;
                flex-wrap: wrap;
                flex-direction: row;
                gap: 1rem;
            }
        }
        .content-block {
            background-color: azure;
            flex-basis: 500px;
            height: fit-content;
            padding: 10px;
            border: 2px solid grey;
            text-align: center;
            position: relative;
        }
        .content-block input {
            border:2px solid grey;
            background-color: azure;
        }
        .hidable {
            display: none;
        }
        .update-button {
            position: absolute;
            top: 5px;
            right: 5px;
            background:rgb(107, 107, 240);
        }
        .btn-group button {
            background-color: #04AA6D; 
            border: 1px solid green;
            padding: 10px 24px;
            cursor: pointer;
            width: 100%;
            display: block;
            margin-top: 10px;
        }
        .btn-group button:not(:last-child) {
            border-bottom: none;
        }
        .btn-restart {
            background-color: rgb(175, 53, 53); 
            border: 1px solid rgb(175, 53, 53);
            margin-top: 10px;
            width: 100%;
        }
        .config-block {
            display:flex;
            flex-direction:row;
            border:2px solid grey;
        }
        .config-block input {
            flex-grow:2;
            border: none;
        }
        .config-block input:focus {
            outline: none;
        }
        .config-block button {
            border:1px solid black;
        }
        .grid-view {
            display: grid;
            align-items: center;
        }
        .grid-view p {
            grid-column: 1;
        }
        .grid-view input, div {
            grid-column: 2;
            margin: 5px;
        }
    </style>
</html>
)=====";