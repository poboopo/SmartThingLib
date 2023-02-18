const String SETUP_PAGE = R"=====(
    <html>
    <head>
        <title>Louver set up page</title>
    </head>
    <body class="center">
        <div>
            <p>WiFi network name: </p>
            <input type="text" id="ssid" title="SSID"/>
        </div>
        <div>
            <p>WiFi password: </p>
            <input type="password" id="password" title="password"/>
        </div>
        <div>
            <button onclick="submit()">submit</button>
        </div>
        <p id="info"></p>
    </body>
    <script>
        function submit() {
            const ssid = document.getElementById("ssid").value
            if (!ssid || !ssid.length) {
                alert("SSID is missing!");
                return;
            }
            const pass = document.getElementById("password").value   
            
            let xhr = new XMLHttpRequest();
            xhr.open("POST", "http://192.168.4.1/settings");
            xhr.setRequestHeader("Accept", "application/json");
            xhr.setRequestHeader("Content-Type", "application/json");

            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4) {
                    if (xhr.status == 200) {
                        document.getElementById("info").innerHTML = "WiFi info saved!"
                    } else {
                        document.getElementById("info").innerHTML = "Can't save WiFi info :("
                    }
                }
            };

            const data = {
                wssid: ssid,
                wpass: pass
            }

            xhr.send(JSON.stringify(data));
        }
    </script>
    <style>
        .center {
            margin: auto;
            width: fit-content;
            height: fit-content;
            border: 3px solid green;
            padding: 10px;
            text-align: center;
        }
    </style>
</html>
)=====";

const String SETUP_BLOCK = R"=====(
    <h1>Settings</h1>
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

const String CONTROLS_BLOCK = R"=====(
    <h1>Control panel</h1>
    <div id="control-buttons-block" class="content-block btn-group btn-control">
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
)=====";

const String PAGE_PART_1 = R"=====(
<html>
    <head>
        <title>Louver control page</title>
    </head>
    <body>
        <div class="holder">
            <!-- yikes -->    
            <h2 id="info-failure" style="color: red" class="center"></h2>
            <h2 id="info-success" style="color: green" class="center"></h2>
        </div>
        <div class="holder">
)=====";

const String PAGE_PART_2 = R"=====(          
        </div>
        <div class="holder">
            <button class="btn-warning" onclick="restart()">Restart</button>
        </div>
    </body>
    <script>
        window.onload = function() {
            loadLouverState();
        };
        function submit() {
            const ssid = document.getElementById("ssid").value;
            if (!ssid || !ssid.length) {
                displayFailure("SSID is missing!");
                return;
            }
            const pass = document.getElementById("password").value;
            restRequest(
                "POST",
                "http://" + getHost() + "/settings",
                { wssid: ssid, wpass: pass },
                "WiFi info saved!",
                "Can't save WiFi info :("
            );
        }
        function sendLouverAction(action) {
            restRequest(
                "PUT",
                "http://" + getHost() + "/louver",
                { action },
                "Done",
                "Failed to perform action",
                function () {
                    loadLouverState();
                }
            );
        }
        function loadLouverState() {
            restRequest(
                "GET",
                "http://" + getHost() + "/louver",
                null,
                "",
                "Failed to load louver state!",
                function (response) {
                    if (response) {
                        const data = JSON.parse(response);
                        this.louverState = data;
                        // BRUHHHH
                        this.automode = this.louverState.automode === "1"
                        updateFields();
                    } else {
                        console.log("Empty response");
                    }
                }
            );
        }
        function updateFields() {
            document.getElementById("auto-mode-checkbox").checked = this.automode
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
            let xhr = new XMLHttpRequest();
            xhr.open(method, path);
            xhr.setRequestHeader("Accept", "application/json");
            xhr.setRequestHeader("Content-Type", "application/json");
            xhr.onreadystatechange = function () {
                    if (xhr.readyState === 4) {
                        if (xhr.status === 200) {
                            if (successText) displaySuccess(successText);
                            if (callback) callback(xhr.response);
                        } else {
                            if (failureText) displayFailure(failureText);
                        }
                    }
                };
            xhr.send(data ? JSON.stringify(data) : null);
        }
        function displayFailure(text) {
            document.getElementById("info-failure").innerHTML = text;
        }
        function displaySuccess(text) {
            document.getElementById("info-success").innerHTML = text;
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
            width: 20%;
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