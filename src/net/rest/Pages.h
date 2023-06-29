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
                <div id="state-fields-block" style="text-align: start;"></div>
            </div>
            <div id="sensors" class="content-block hidable">
                <div class="loading-info">Loading</div>
                <h1>Sensors values</h1>
                <button class="update-button" onclick="loadSensors()">Update</button>
                <div id="sensors-fields-block" style="text-align: start;"></div>
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
                    const p = document.createElement("p");
                    p.innerHTML = key;
                    block.appendChild(p);
                    const input = document.createElement("input");
                    input.value = value;
                    input.id = block.id + "-" + key;
                    if (key === "name") {
                        input.title = "Insert new name";
                        input.disabled = false;
                        const button = document.createElement("button");
                        button.style.backgroundColor = "#04AA6D";
                        button.innerHTML = "Save";
                        button.title = "Save new device name";
                        button.onclick = () => saveNewName();
                        const div = document.createElement("div");
                        div.className = "config-block";
                        div.appendChild(input);
                        div.appendChild(button);
                        block.appendChild(div);
                    } else {
                        input.disabled = true;
                        block.appendChild(input);
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
                            document.getElementById("ssid").value = data["settings"]["ss"];
                            document.getElementById("password").value = data["settings"]["ps"];
                            fillComboBox("wifi-mode", data["modes"], data["settings"]["md"]);
                        }
                    }
                },
                "wifi"
            );
        }
        function fillComboBox(comboboxId, values, selectedValue) {
            console.log(values);
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
                            Object.entries(loadedConfig).forEach(([key, value]) => this.config[key] = value);
                        }
                        updateConfigFields();
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
        function updateConfigFields() {
            if (this.config) {
                Object.keys(this.config).forEach((key) => document.getElementById(key).value = this.config[key]);
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
                        updateBlockValues("state-fields-block", JSON.parse(response));
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
                        updateBlockValues("sensors-fields-block", JSON.parse(response));
                    }
                },
                "sensors"
            );
        }
        function updateBlockValues(blockId, elements) {
            if (!elements) {
                return;
            }
            const stateBlock = document.getElementById(blockId);
            stateBlock.innerHTML = "";
            Object.entries(elements).forEach(([key, value]) => {
                const p = document.createElement("p");
                p.id = key;
                p.innerHTML = key + ": " + value;
                stateBlock.appendChild(p);
            });
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
            const actions = this.dictionaries.actions;
            if (actions) {
                document.getElementById("actions").style.display = "block";
                const actionsBlock = document.getElementById("control-buttons-block");
                actions.forEach((action) => {
                    const button = document.createElement("button");
                    button.onclick = function() {
                        if (action.action || action.action == 0) {
                            restRequest("PUT", "http://" + getHost() + "/action?action=" + action.action);
                        } else {
                            console.error("Action is missing!");
                        }
                    };
                    button.innerHTML = action.caption;
                    actionsBlock.appendChild(button);
                });
            }

            const configFields = this.dictionaries.config;
            if (configFields) {
                document.getElementById("config").style.display = "block";
                const configFieldsBlock = document.getElementById("config-fields-block");
                configFields.forEach((configField) => {
                    this.config[configField.name] = null;
                    const p = document.createElement("p");
                    p.innerHTML = configField.caption;
                    p.for = configField.name;
                    const input = document.createElement("input");
                    input.type = configField.type;
                    input.id = configField.name;
                    const button = document.createElement("button");
                    button.innerHTML = "X";
                    button.title = "Clear config value";
                    button.style.backgroundColor = "rgb(175, 53, 53)";
                    button.onclick = function () {
                        deleteConfigValue(configField.name);
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
            // return "192.168.1.104";
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
                            if (callback) callback(xhr.response);
                            if (blockId) displayBlockInfo(blockId, false);
                        } else {
                            console.error("Request " + path + " failed with code " + xhr.status);
                            if (blockId) displayBlockInfo(blockId, true, "Failed", "rgb(175, 53, 53)");
                        }
                    }
                };
            xhr.send(data ? JSON.stringify(data) : null);
        }
    </script>
    <style>
        body {
            background-color: aliceblue;
        }
        * {
            border-radius: 20px;
        }
        p, input, button, select, .loading-info {
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
            p, input, button, select, .loading-info {
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
            flex-basis: 400px;
            height: fit-content;
            padding: 10px;
            border: 2px solid grey;
            text-align: center;
            position: relative;
        }
        .content-block input {
            border:2px solid grey;
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