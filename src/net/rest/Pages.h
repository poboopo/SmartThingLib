const String WEB_PAGE_MAIN = R"=====(
<html>
    <head>
        <title>SmartThing control page</title>
    </head>
    <body>
        <div class="info-list">
            <label for="info">Logs:</label>
            <lu id="info"></lu>
        </div>
        <div id="main-panel" class="holder">
            <div id="settings" class="content-block">
                <h1>WiFi settings</h1>
                <div>
                    <p>WiFi network name: </p>
                    <input type="text" id="ssid" title="SSID"/>
                </div>
                <div>
                    <p>WiFi password: </p>
                    <input type="password" id="password" title="password"/>
                </div>
                <div>
                    <p>WiFi mode (1-2): </p>
                    <input type="number" id="wifi-mode" title="mode" value="1"/>
                </div>
                <div class="btn-group btn-control" >
                    <button onclick="saveWifiSettings()">Save and reconnect</button>
                </div>
            </div>
            <div id="actions" class="content-block btn-group btn-control">
                <h1>Control panel</h1>
                <div id="control-buttons-block"></div>
            </div>
            <div id="config" class="content-block">
                <h1>Config</h1>
                <div id="config-fields-block"></div>
                <div class="btn-group btn-control" >
                    <button title="Save config values" onclick="saveConfig()">save</button>
                </div>
                <button class="update-button" onclick="loadConfig()">Update</button>
            </div>
            <div id="state" class="content-block values-block">
                <h1>State</h1>
                <button class="update-button" onclick="loadState()">Update</button>
                <div id="state-fields-block"></div>
            </div>
            <div id="sensors" class="content-block values-block">
                <h1>Sensors values</h1>
                <button class="update-button" onclick="loadSensors()">Update</button>
                <div id="sensors-fields-block"></div>
            </div>
            <button title="Restart device" class="btn-warning" onclick="restart()">Restart</button>
        </div>
    </body>
    <script>
        this.config = {};
        this.dictionaries = {};
        this.infoStyles = {
            error: 'color: red',
            success: 'color: green',
            warning: 'color: orange'
        }

        window.onload = function() {
            loadWiFiSettings();
            processDictionaries();
            loadDictionaries();
            loadState();
            loadSensors();
        };

        function saveWifiSettings() {
            const ssid = document.getElementById("ssid").value;
            if (!ssid || !ssid.length) {
                info({text: "SSID is missing!", type: 'error', id: 'wifi'});
                return;
            }
            const pass = document.getElementById("password").value;
            const mode = document.getElementById("wifi-mode").value;
            restRequest(
                "POST",
                "http://" + getHost() + "/wifi",
                { ssid: ssid, password: pass, mode: mode },
                "WiFi info saved!",
                "Can't save WiFi info :("
            );
        }
        function loadWiFiSettings() {
            restRequest(
                "GET",
                "http://" + getHost() + "/wifi",
                null,
                "WiFi info loaded",
                "Failed to load WiFi info!",
                function (response) {
                    if (response) {
                        response.trim();
                        const data = JSON.parse(response);
                        if (data["settings"]) {
                            document.getElementById("ssid").value = data["settings"]["ss"];
                            document.getElementById("password").value = data["settings"]["ps"];
                            document.getElementById("wifi-mode").value = data["settings"]["md"];
                        }
                    }
                }
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
                "Config updated!",
                "Failed to update config!",
                null
            );
        }
        function loadState() {
            restRequest(
                "GET",
                "http://" + getHost() + "/state",
                null,
                "State loaded",
                "Failed to load state!",
                function (response) {
                    if (response) {
                        response.trim();
                        updateBlockValues("state-fields-block", JSON.parse(response));
                    }
                }
            );
        }
        function loadSensors() {
            restRequest(
                "GET",
                "http://" + getHost() + "/sensors",
                null,
                "Sensors values loaded",
                "Failed to load sensors values!",
                function (response) {
                    if (response) {
                        response.trim();
                        updateBlockValues("sensors-fields-block", JSON.parse(response));
                    }
                }
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
        function loadConfig() {
            restRequest(
                "GET",
                "http://" + getHost() + "/config",
                null,
                "Config loaded",
                "Failed to config!",
                function (response) {
                    if (response) {
                        const loadedConfig = JSON.parse(response);
                        if (loadedConfig) {
                            Object.entries(loadedConfig).forEach(([key, value]) => this.config[key] = value);
                        }
                        updateConfigFields();
                    }
                }
            );
        }
        function deleteConfigValue(name) {
            if (name) {
                restRequest(
                    "DELETE",
                    "http://" + getHost() + "/config?name=" + name,
                    null,
                    "Value deleted",
                    "Failed to delete config value",
                    function(response) {
                        document.getElementById(name).value = null;
                    }
                )
            }
        }
        function updateConfigFields() {
            if (this.config) {
                Object.keys(this.config).forEach((key) => document.getElementById(key).value = this.config[key]);
            }
        }
        function processDictionaries() {
            const actions = this.dictionaries.actions;
            if (actions) {
                const actionsBlock = document.getElementById("control-buttons-block");
                actions.forEach((action) => {
                    const button = document.createElement("button");
                    button.onclick = function() {
                        if (action.action || action.action == 0) {
                            restRequest("PUT", "http://" + getHost() + "/action?action=" + action.action,
                                null, "Done", "Failed to perform action '" + action.caption + "'", null);
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
        function getHost() {
            const { host } = window.location;
            return host;
        }
        function loadDictionaries() {
            restRequest(
                "GET",
                "http://" + getHost() + "/dictionary",
                null,
                "Dictionaries loaded",
                "Failed to load dictionaries!",
                function(response) {
                    if (response) {
                        this.dictionaries = JSON.parse(response);
                        processDictionaries();
                        loadConfig();
                    }
                }
            )
        }
        function restRequest(method, path, data, successText, failureText, callback) {
            info({text: "Processing...", id: path});
            let xhr = new XMLHttpRequest();
            xhr.open(method, path);
            xhr.setRequestHeader("Accept", "application/json");
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4) {
                        if (xhr.status === 200) {
                            if (successText) info({text: successText, id: path});
                            if (callback) callback(xhr.response);
                        } else {
                            if (failureText) info({text: failureText, type: 'error', id: path});
                        }
                    }
                };
            xhr.send(data ? JSON.stringify(data) : null);
        }
        function info({text, type = 'success', id = '1'}) {
            const infoElement = document.getElementById("info");
            if (infoElement) {
                let newElement = false;
                let element = document.getElementById(id);
                if (!element) {
                    element = document.createElement("li")
                    newElement = true;
                }
                element.innerHTML = text;
                element.style = this.infoStyles[type];
                element.id = id;
                if (newElement) {
                    infoElement.appendChild(element);
                }
            }
        }
    </script>
    <style>
        .holder {
            text-align: center;
            margin-left: auto;
            margin-right: auto;
            width: 50%;
        }
        .content-block {
            margin-top: 10px;
            margin-bottom: 10px;
            padding: 10px;
            width: 100%;
            height: fit-content;
            border: 2px solid grey;
            text-align: center;
            border-radius: 8px;
            position: relative;
        }
        .content-block input, p {
            font-size: 18px;
        }
        .update-button {
            position: absolute;
            top: 5px;
            right: 5px;
            background:rgb(107, 107, 240);
            border-radius: 8px;
            width: fit-content;
            height: fit-content;
        }
        .btn-group button {
            background-color: #04AA6D; 
            border: 1px solid green;
            padding: 10px 24px;
            cursor: pointer;
            width: 100%;
            display: block;
            border-radius: 8px;
            margin-top: 10px;
            font-size: 18px;
        }
        .btn-group button:not(:last-child) {
            border-bottom: none; /* Prevent double borders */
        }
        .btn-warning {
            background-color: rgb(175, 53, 53); 
            border: 1px solid red;
            margin-top: 10px;
        }
        .config-block {
            display:flex;
            flex-direction:row;
            border:2px solid grey;
            border-radius: 8px;
            padding:1px;
        }
        .config-block input {
            flex-grow:2;
            border:none;
            width: 100%;
            height: 25px;
        }
        .config-block input:focus {
            outline: none;
        }
        .config-block button {
            border:1px solid black;
            background:red;
            color:white;
            border-radius: 8px;
        }
        .info-list {
            position: absolute;
            left: 10px;
            top: 10px;
            width: 20%;
            font-size: 25px;
        }
    </style>
</html>
)=====";