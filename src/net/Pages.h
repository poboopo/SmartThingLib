const String SETUP_BLOCK = R"=====(
    <h1>WiFi setup</h1>
    <div id="settings" class="content-block">
        <div>
            <p>WiFi network name: </p>
            <input type="text" id="ssid" title="SSID"/>
        </div>
        <div>
            <p>WiFi password: </p>
            <input type="password" id="password" title="password"/>
        </div>
        <div class="btn-group btn-control" >
            <button onclick="submit()">submit</button>
        </div>
    </div>
)=====";

const String PAGE_PART_1 = R"=====(
<html>
    <head>
        <title>Louver control page</title>
    </head>
    <body>
        <div class="holder">
            <h2 id="info" class="center"></h2>
        </div>
        <div class="holder">
)=====";

const String PAGE_PART_2 = R"=====(
        <div id="control-buttons-block" class="content-block btn-group btn-control">
                <h1>Control panel</h1>
                <div>
                    <p style="display: inline-block; padding-right: 10px;">Auto mode</p>
                    <label class="switch"  style="vertical-align: 13px;">
                        <!-- + true конвертит в int -->
                        <input type="checkbox" id="auto-mode-checkbox" onclick="sendLouverAction(+ this.checked)">
                        <span class="slider round"></span>
                    </label>
                </div>
        
                <button onclick="sendLouverAction(3)">Close</button>
                <button onclick="sendLouverAction(4)">Middle</button>
                <button onclick="sendLouverAction(2)">Open</button>
                <button onclick="sendLouverAction(5)">Bright</button>
            </div>

            <div class="content-block">
                <h1>Config</h1>
                <div id="config">
                    <p>Light close value</p>
                    <input type="number" id="light_close">
                    <p>Light open value</p>
                    <input type="number" id="light_open">
                    <p>Light bright value</p>
                    <input type="number" id="light_bright">
                    <p>Automode update delay</p>
                    <input type="number" id="delay">
                    <p>Motor accuracy</p>
                    <input type="number" id="accuracy">
                </div>
                <div class="btn-group btn-control" >
                    <button onclick="saveConfig()">save</button>
                </div>
            </div>

            <div class="content-block">
                <h1>State</h1>
                <p style="display: inline-block; padding-right: 10px;" id="state">Loading...</p>
                <button onclick="loadLouverState()">Update</button>
            </div>
            <button class="btn-warning" onclick="restart()">Restart</button>
        </div>
    </body>
    <script>
        this.config = {};
        this.louverState = {};
        this.infoStyles = {
            error: 'color: red',
            success: 'color: green' 
        }

        window.onload = function() {
            loadConfig();
            loadLouverState();
        };

        function submit() {
            const ssid = document.getElementById("ssid").value;
            if (!ssid || !ssid.length) {
                info("SSID is missing!", 'error');
                return;
            }
            const pass = document.getElementById("password").value;
            restRequest(
                "POST",
                "http://" + getHost() + "/setup",
                { ssid: ssid, password: pass },
                "WiFi info saved!",
                "Can't save WiFi info :("
            );
        }
        function saveConfig() {
            if(!this.config) {
                this.config = {};
            }
            const configElement = document.getElementById("config");
            for (let i = 0; i < configElement.children.length; i++) {
                const element = configElement.children[i];
                if (element.localName === 'input' && element.id) {
                    if (element.type === 'number') {
                        this.config[element.id] = parseInt(element.value);
                    } else {
                        this.config[element.id] = element.value;
                    }
                }
            }
            restRequest(
                "POST",
                "http://" + getHost() + "/settings",
                this.config,
                "Config updated!",
                "Failed to update config!",
                null
            );
        }
        function sendLouverAction(action) {
            if (action > 1) {
                this.louverState.automode = false;
                const checkbox = document.getElementById('auto-mode-checkbox');
                if (checkbox) {
                    checkbox.checked = false;
                }
            }
            restRequest(
                "PUT",
                "http://" + getHost() + "/louver",
                { action },
                "Done",
                "Failed to perform action",
                null
            );
        }
        function loadLouverState() {
            restRequest(
                "GET",
                "http://" + getHost() + "/louver",
                null,
                "Louver state loaded",
                "Failed to load louver state!",
                function (response) {
                    if (response) {
                        this.louverState = JSON.parse(response);
                        updateFields();
                    } else {
                        console.log("Empty response");
                    }
                }
            );
        }
        function loadConfig() {
            restRequest(
                "GET",
                "http://" + getHost() + "/settings",
                null,
                "Config loaded",
                "Failed to load louver config!",
                function (response) {
                    if (response) {
                        this.config = JSON.parse(response);
                        updateConfigFields();
                    } else {
                        console.log("Empty response");
                    }
                }
            );
        }
        function updateFields() {
            if (this.louverState) {
                document.getElementById("auto-mode-checkbox").checked = this.louverState.automode;
                document.getElementById("state").innerHTML = JSON.stringify(this.louverState, null, 4);
            }
        }
        function updateConfigFields() {
            if (this.config) {
                Object.keys(this.config).forEach((key) => document.getElementById(key).value = this.config[key]);
            }
        }
        function restart() {
            restRequest("PUT", "http://" + getHost() + "/restart");
        }
        function getHost() {
            const { host } = window.location;
            if (!host) {
                return "192.168.0.105";
            }
            return host;
        }
        function restRequest(method, path, data, successText, failureText, callback) {
            info("Processing...");
            let xhr = new XMLHttpRequest();
            xhr.open(method, path);
            xhr.setRequestHeader("Accept", "application/json");
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4) {
                        if (xhr.status === 200) {
                            if (successText) info(successText);
                            if (callback) callback(xhr.response);
                        } else {
                            if (failureText) info(failureText, 'error');
                        }
                    }
                };
            xhr.send(data ? JSON.stringify(data) : null);
        }
        function info(text, type = 'success') {
            const infoElement = document.getElementById("info");
            if (infoElement) {
                infoElement.innerHTML = text;
                infoElement.style = this.infoStyles[type];
            }
        }
    </script>
    <style>
        input {
            border-radius: 8px;
            width: 100%;
            height: 25px;
        }
        .holder {
            text-align: center;
            margin-left: auto;
            margin-right: auto;
            width: 50%;
        }
        .content-block {
            margin-top: 10px;
            margin-left: auto;
            margin-right: auto;
            width: 100%;
            height: fit-content;
            /* border: 3px solid black; */
            text-align: center;
            border-radius: 8px;
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
        }
        .btn-group button:not(:last-child) {
            border-bottom: none; /* Prevent double borders */
        }
        .btn-warning {
            background-color: rgb(175, 53, 53); 
            border: 1px solid red;
            margin-top: 10px;
        }
        /* The switch - the box around the slider */
        .switch {
            position: relative;
            display: inline-block;
            width: 54px;
            height: 28px;
            margin-bottom: 10px;
        }
        /* Hide default HTML checkbox */
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        /* The slider */
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            -webkit-transition: .4s;
            transition: .4s;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 20px;
            width: 20px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            -webkit-transition: .4s;
            transition: .4s;
        }
        input:checked + .slider {
            background-color: green;
        }
        input:focus + .slider {
            box-shadow: 0 0 1px green;
        }
        input:checked + .slider:before {
            -webkit-transform: translateX(26px);
            -ms-transform: translateX(26px);
            transform: translateX(26px);
        }
        /* Rounded sliders */
        .slider.round {
            border-radius: 34px;
        }
        .slider.round:before {
            border-radius: 50%;
        }
    </style>
</html>
)=====";