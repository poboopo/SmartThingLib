const char * SETUP_PAGE = R"=====(
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

const char * GREETING_PAGE = R"=====(
<html>
    <head>
        <title>Louver</title>
    </head>
    <body class="center">
        <p>Hi there</p>
    </body>
    <style>
        .center {
            margin: auto;
            width: fit-content;
            height: fit-content;
            padding: 10px;
            text-align: center;
        }
    </style>
</html>
)=====";