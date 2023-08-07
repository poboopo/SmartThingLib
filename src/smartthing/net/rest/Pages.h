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
                </div>
                <button class="update-button" onclick="loadConfig()">Update</button>
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
                loadCallbacks(callbacksBlock.getAttribute("type"), callbacksBlock.getAttribute("observable"));
            }
        }
        function loadCallbacks(type, observable) {
            if (!type || !observable) {
                console.warn("Watcher type or observable are missing!");
                return;
            }
            restRequest(
                "GET",
                "http://" + getHost() + "/callbacks?type=" + type + "&name=" + observable,
                null,
                function (response) {
                    if (response) {
                        displayCallbacks(type, observable, JSON.parse(response));
                    }
                },
                "callbacks"
            );
        }

        function addCallback(type, observable) {
            if (document.getElementById("callback_new")) {
                return;
            }
            const callbacksBlock = document.getElementById("callbacks-block");
            if (!callbacksBlock) {
                console.error("Can't find callbacks-block!");
                return;
            }
            callbacksBlock.prepend(
                buildCallbackView(type, observable, {
                    "type": "http_callback",
                    "readonly": false, 
                    "url": "callback url",
                    "caption": "New http callback"
                }, "new")
            );
        }   
        function displayCallbacks(type, observable, callbacks) {
            const callbacksBlock = document.getElementById("callbacks-block");
            if (!callbacksBlock) {
                console.error("Can't find callbacks-block!");
                return;
            }
            callbacksBlock.setAttribute("type", type);
            callbacksBlock.setAttribute("observable", observable);
            callbacksBlock.innerHTML = "";
            document.getElementById("callbacks").style.display = "block";

            if (callbacks) {
                callbacks.forEach((callback, index) => {
                    callbacksBlock.appendChild(buildCallbackView(type, observable, callback, index));
                });
            }   else {
                console.warn("Callbacks list are empty!");
            }

            const addButton = document.getElementById("add-new-callback");
            if (addButton) {
                addButton.onclick = () => {
                    addCallback(type, observable);  
                };
            }
            document.getElementById("callbacks-title").innerHTML = type + " " + observable + "'s callbacks";
        }
        function buildCallbackView(type, observable, callback, index) {
            const div = document.createElement("div");
            div.id = "callback_" + index;
            div.className = "content-block";
            const h2 = document.createElement("h2");
            h2.innerHTML = callback["caption"] || callback["type"];
            div.appendChild(h2);
            const block = document.createElement("div");
            block.className = "grid-view";
            if (callback["type"] == "http_callback") {
                createEntryInput(
                    block,
                    "callback_url_" + index,
                    "Url",
                    callback.url,
                    "Callback url",
                    callback.readonly
                );
                createEntryInput(
                    block,
                    "callback_response_" + index,
                    "Last callback response code",
                    callback.lastResponseCode || "Not called yet",
                    "Last callback response code",
                    true
                );
            }
            createEntryInput(
                block,
                "callback_trigger_" + index,
                "Trigger value",
                (callback.trigger || callback.trigger == 0) ? callback.trigger : "",
                "Call callback when value equals",
                callback.readonly
            );
            div.appendChild(block);

            if (!callback.readonly) {
                const wrap = document.createElement("div");
                wrap.classList.add("btn-group", "grid-view");
                const button = document.createElement("button");
                button.innerHTML = "save"
                button.onclick = () => saveCallback(type, observable, index);
                wrap.appendChild(button);
                if (index != "new") {
                    const deleteButton = document.createElement("button");
                    deleteButton.innerHTML = "delete";
                    deleteButton.style.backgroundColor = "red";
                    deleteButton.onclick = () => {
                        if (confirm("Are you sure you want to delete this callback?")) {
                            deleteCallback(type, observable, index);
                        }
                    };
                    wrap.appendChild(deleteButton);
                }
                div.appendChild(wrap);
            }
            return div;
        }
        function saveCallback(type, observable, index) {
            const reqPayload = {type, observable};
            const triggerInput = document.getElementById("callback_trigger_" + index);
            if (triggerInput) {
                reqPayload["trigger"] = triggerInput.value;
            }
            const urlInput = document.getElementById("callback_url_" + index);
            if (urlInput) {
                reqPayload["url"] = urlInput.value;
            }

            if (Object.keys(reqPayload).length === 0) {
                console.error("Empty update payload! Skipping");
                return;
            }
            
            if (index == "new") {
                restRequest(
                    "POST",
                    "http://" + getHost() + "/callbacks",
                    reqPayload,
                    (response) => {
                        console.log("Callback created!");
                        loadCallbacks(type, observable);
                    },
                    "callbacks"
                )
            } else {
                const queryPart = "?type=" + type + "&name=" + observable + "&index=" + index;
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
        function deleteCallback(type, observable, index) {
            const queryPart = "?type=" + type + "&name=" + observable + "&index=" + index;
            restRequest(
                "DELETE",
                "http://" + getHost() + "/callbacks" + queryPart,
                null,
                (response) => {
                    console.log("Callback deleted :(");
                    loadCallbacks(type, observable);
                },
                "callbacks"
            )
        }
        function loadDeviceInfo() {
            restRequest(
                "GET",
                "http://" + getHost() + "/info",
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
        function loadConfig() {
            restRequest(
                "GET",
                "http://" + getHost() + "/config",
                null,
                function (response) {
                    if (response) {
                        const loadedConfig = JSON.parse(response);
                        if (loadedConfig) {
                            Object.entries(loadedConfig).forEach(([key, value]) => {
                                this.config[key] = value;
                                document.getElementById(key).value = value;
                            });
                        }
                    }
                },
                "config"
            );
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
                    "http://" + getHost() + "/config?name=" + name,
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
                "http://" + getHost() + "/dictionary",
                null,
                function(response) {
                    if (response) {
                        this.dictionaries = JSON.parse(response);
                        processDictionaries();
                        loadConfig();
                    }
                }
            )
        }
        function processDictionaries() {
            if (!this.dictionaries) {
                return;
            }
            const actions = this.dictionaries.actions;
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

            const configFields = this.dictionaries.config;
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
            // return "192.168.63.17";
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