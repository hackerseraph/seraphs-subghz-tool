#include "wifi_ap.h"
#include "subghz_operations.h"
#include "menu_system.h"
#include <M5StickCPlus.h>

WiFiAP::WiFiAP(SubGhzOperations* ops, MenuSystem* menu) {
    operations = ops;
    menuSystem = menu;
    server = nullptr;
    active = false;
    lastStatusUpdate = 0;
    recordingActive = false;
    recordedSignalData = "";
}

void WiFiAP::begin() {
    if (active) return;
    
    // Configure Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP("roku-hd", "");  // No password for easy access
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    // Create web server
    server = new WebServer(80);
    
    // Register handlers
    server->on("/", [this]() { handleRoot(); });
    server->on("/listen", HTTP_POST, [this]() { handleListen(); });
    server->on("/record", HTTP_POST, [this]() { handleRecord(); });
    server->on("/replay", HTTP_POST, [this]() { handleReplay(); });
    server->on("/status", HTTP_GET, [this]() { handleStatus(); });
    server->on("/stop", HTTP_POST, [this]() { handleStop(); });
    server->onNotFound([this]() { handleNotFound(); });
    
    server->begin();
    active = true;
    
    Serial.println("WiFi AP started: roku-hd");
    Serial.println("Web interface available at http://192.168.4.1");
}

void WiFiAP::update() {
    if (!active || server == nullptr) return;
    
    server->handleClient();
    
    // Update status display periodically
    if (millis() - lastStatusUpdate > 1000) {
        lastStatusUpdate = millis();
    }
}

void WiFiAP::stop() {
    if (!active) return;
    
    if (server != nullptr) {
        server->stop();
        delete server;
        server = nullptr;
    }
    
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    active = false;
    recordingActive = false;
    
    Serial.println("WiFi AP stopped");
}

bool WiFiAP::isActive() {
    return active;
}

String WiFiAP::getIPAddress() {
    if (!active) return "N/A";
    return WiFi.softAPIP().toString();
}

int WiFiAP::getClientCount() {
    if (!active) return 0;
    return WiFi.softAPgetStationNum();
}

void WiFiAP::handleRoot() {
    String page = getWebPage();
    server->send(200, "text/html", page);
}

void WiFiAP::handleListen() {
    if (server->hasArg("frequency")) {
        float freq = server->arg("frequency").toFloat();
        
        // Set frequency based on argument
        int freqIndex = 1; // Default 433MHz
        if (freq >= 310 && freq <= 320) freqIndex = 0;
        else if (freq >= 430 && freq <= 440) freqIndex = 1;
        else if (freq >= 860 && freq <= 875) freqIndex = 2;
        else if (freq >= 910 && freq <= 920) freqIndex = 3;
        
        // Set mode to listening
        menuSystem->setMode(MODE_LISTENING);
        
        server->send(200, "application/json", "{\"status\":\"listening\",\"frequency\":" + String(freq) + "}");
    } else {
        server->send(400, "application/json", "{\"error\":\"Missing frequency parameter\"}");
    }
}

void WiFiAP::handleRecord() {
    if (server->hasArg("frequency")) {
        float freq = server->arg("frequency").toFloat();
        
        // Set frequency
        int freqIndex = 1;
        if (freq >= 310 && freq <= 320) freqIndex = 0;
        else if (freq >= 430 && freq <= 440) freqIndex = 1;
        else if (freq >= 860 && freq <= 875) freqIndex = 2;
        else if (freq >= 910 && freq <= 920) freqIndex = 3;
        
        // Start recording
        menuSystem->setMode(MODE_RECORDING);
        recordingActive = true;
        
        server->send(200, "application/json", "{\"status\":\"recording\",\"frequency\":" + String(freq) + "}");
    } else {
        server->send(400, "application/json", "{\"error\":\"Missing frequency parameter\"}");
    }
}

void WiFiAP::handleReplay() {
    if (menuSystem->getMode() == MODE_RECORDING) {
        server->send(400, "application/json", "{\"error\":\"Still recording, cannot replay yet\"}");
        return;
    }
    
    if (server->hasArg("frequency")) {
        float freq = server->arg("frequency").toFloat();
        
        // Set mode to replay
        menuSystem->setMode(MODE_REPLAYING);
        
        server->send(200, "application/json", "{\"status\":\"replaying\",\"frequency\":" + String(freq) + "}");
    } else {
        server->send(400, "application/json", "{\"error\":\"Missing frequency parameter\"}");
    }
}

void WiFiAP::handleStatus() {
    String status = getCurrentStatus();
    server->send(200, "application/json", status);
}

void WiFiAP::handleStop() {
    // Stop current operation
    menuSystem->setMode(MODE_IDLE);
    recordingActive = false;
    
    server->send(200, "application/json", "{\"status\":\"stopped\"}");
}

void WiFiAP::handleNotFound() {
    server->send(404, "text/plain", "Not found");
}

String WiFiAP::getWebPage() {
    String html = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta charset="UTF-8">
    <title>M5 RF Tools - Remote Control</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Courier New', monospace;
            background: #0d0d0d;
            min-height: 100vh;
            padding: 20px;
            color: #00ff00;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: #1a1a1a;
            border: 2px solid #00ff00;
            border-radius: 5px;
            padding: 30px;
            box-shadow: 0 0 20px rgba(0, 255, 0, 0.3);
        }
        h1 {
            text-align: center;
            margin-bottom: 10px;
            font-size: 2em;
            color: #00ff00;
            text-shadow: 0 0 10px #00ff00;
        }
        .subtitle {
            text-align: center;
            margin-bottom: 30px;
            color: #00cc00;
            font-size: 0.9em;
        }
        .status-panel {
            background: #0d0d0d;
            border: 1px solid #00ff00;
            border-radius: 5px;
            padding: 20px;
            margin-bottom: 30px;
        }
        .status-item {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            padding: 8px 0;
            border-bottom: 1px solid #003300;
        }
        .status-item:last-child {
            border-bottom: none;
            margin-bottom: 0;
        }
        .status-label {
            color: #00aa00;
        }
        .status-value {
            font-weight: bold;
            color: #00ff00;
        }
        .control-section {
            margin-bottom: 25px;
        }
        .control-section h2 {
            font-size: 1.2em;
            margin-bottom: 15px;
            color: #00ff00;
        }
        .frequency-selector {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 10px;
            margin-bottom: 15px;
        }
        .freq-btn {
            padding: 12px;
            background: #2a2a2a;
            border: 2px solid #004400;
            border-radius: 3px;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            font-size: 1em;
            cursor: pointer;
            transition: all 0.2s;
        }
        .freq-btn:hover {
            background: #333333;
            border-color: #00ff00;
            box-shadow: 0 0 10px rgba(0, 255, 0, 0.3);
        }
        .freq-btn.active {
            background: #003300;
            border-color: #00ff00;
            box-shadow: 0 0 15px rgba(0, 255, 0, 0.5);
        }
        .action-buttons {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
        }
        .action-btn {
            padding: 15px 10px;
            background: #2a2a2a;
            border: 2px solid #00ff00;
            border-radius: 3px;
            color: #00ff00;
            font-family: 'Courier New', monospace;
            font-size: 0.95em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.2s;
        }
        .action-btn:hover {
            background: #003300;
            box-shadow: 0 0 15px rgba(0, 255, 0, 0.4);
        }
        .action-btn:active {
            transform: scale(0.98);
        }
        .action-btn.listen {
            background: #2a2a2a;
        }
        .action-btn.record {
            background: #2a2a2a;
        }
        .action-btn.replay {
            background: #2a2a2a;
        }
        .action-btn.stop {
            background: #1a1a1a;
            border-color: #ff0000;
            color: #ff0000;
            grid-column: span 3;
        }
        .action-btn.stop:hover {
            background: #330000;
            box-shadow: 0 0 15px rgba(255, 0, 0, 0.4);
        }
        .message {
            margin-top: 20px;
            padding: 15px;
            border-radius: 3px;
            text-align: center;
            display: none;
            font-family: 'Courier New', monospace;
        }
        .message.success {
            background: #003300;
            border: 1px solid #00ff00;
            color: #00ff00;
        }
        .message.error {
            background: #330000;
            border: 1px solid #ff0000;
            color: #ff0000;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            color: #006600;
            font-size: 0.85em;
        }
        @media (max-width: 480px) {
            .container {
                padding: 20px;
            }
            h1 {
                font-size: 1.5em;
            }
            .frequency-selector {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Seraph SubGHZ Tool Remote</h1>
        <div class="subtitle">REMOTE SUBGHZ CONTROL PANEL</div>
        
        <div class="status-panel" id="statusPanel">
            <div class="status-item">
                <span class="status-label">Mode:</span>
                <span class="status-value" id="currentMode">Idle</span>
            </div>
            <div class="status-item">
                <span class="status-label">Clients:</span>
                <span class="status-value" id="clientCount">0</span>
            </div>
            <div class="status-item">
                <span class="status-label">IP:</span>
                <span class="status-value">192.168.4.1</span>
            </div>
        </div>
        
        <div class="control-section">
            <h2>FREQUENCY SELECTION</h2>
            <div class="frequency-selector">
                <button class="freq-btn" data-freq="315" onclick="selectFreq(315)">315 MHz</button>
                <button class="freq-btn active" data-freq="433.92" onclick="selectFreq(433.92)">433.92 MHz</button>
                <button class="freq-btn" data-freq="868" onclick="selectFreq(868)">868 MHz</button>
                <button class="freq-btn" data-freq="915" onclick="selectFreq(915)">915 MHz</button>
            </div>
        </div>
        
        <div class="control-section">
            <h2>ACTIONS</h2>
            <div class="action-buttons">
                <button class="action-btn listen" onclick="startListen()">LISTEN</button>
                <button class="action-btn record" onclick="startRecord()">RECORD</button>
                <button class="action-btn replay" onclick="startReplay()">REPLAY</button>
                <button class="action-btn stop" onclick="stopAction()">STOP</button>
            </div>
        </div>
        
        <div id="message" class="message"></div>
        
        <div class="footer">
            Seraph SubGHz Tool v0.4.0
        </div>
    </div>
    
    <script>
        let selectedFrequency = 433.92;
        
        function selectFreq(freq) {
            selectedFrequency = freq;
            document.querySelectorAll('.freq-btn').forEach(btn => {
                btn.classList.remove('active');
            });
            event.target.classList.add('active');
            showMessage('Frequency set to ' + freq + ' MHz', 'success');
        }
        
        function showMessage(text, type) {
            const msgDiv = document.getElementById('message');
            msgDiv.textContent = text;
            msgDiv.className = 'message ' + type;
            msgDiv.style.display = 'block';
            setTimeout(() => {
                msgDiv.style.display = 'none';
            }, 3000);
        }
        
        async function startListen() {
            try {
                const response = await fetch('/listen', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'frequency=' + selectedFrequency
                });
                const data = await response.json();
                if (response.ok) {
                    showMessage('Listening on ' + selectedFrequency + ' MHz', 'success');
                    updateStatus();
                } else {
                    showMessage(data.error || 'Failed to start listening', 'error');
                }
            } catch (error) {
                showMessage('Error: ' + error.message, 'error');
            }
        }
        
        async function startRecord() {
            try {
                const response = await fetch('/record', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'frequency=' + selectedFrequency
                });
                const data = await response.json();
                if (response.ok) {
                    showMessage('Recording on ' + selectedFrequency + ' MHz', 'success');
                    updateStatus();
                } else {
                    showMessage(data.error || 'Failed to start recording', 'error');
                }
            } catch (error) {
                showMessage('Error: ' + error.message, 'error');
            }
        }
        
        async function startReplay() {
            try {
                const response = await fetch('/replay', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                    body: 'frequency=' + selectedFrequency
                });
                const data = await response.json();
                if (response.ok) {
                    showMessage('Replaying on ' + selectedFrequency + ' MHz', 'success');
                    updateStatus();
                } else {
                    showMessage(data.error || 'Failed to replay', 'error');
                }
            } catch (error) {
                showMessage('Error: ' + error.message, 'error');
            }
        }
        
        async function stopAction() {
            try {
                const response = await fetch('/stop', {
                    method: 'POST'
                });
                const data = await response.json();
                if (response.ok) {
                    showMessage('Operation stopped', 'success');
                    updateStatus();
                } else {
                    showMessage('Failed to stop operation', 'error');
                }
            } catch (error) {
                showMessage('Error: ' + error.message, 'error');
            }
        }
        
        async function updateStatus() {
            try {
                const response = await fetch('/status');
                const data = await response.json();
                if (response.ok) {
                    document.getElementById('currentMode').textContent = data.mode || 'Idle';
                    document.getElementById('clientCount').textContent = data.clients || '0';
                }
            } catch (error) {
                console.error('Failed to update status:', error);
            }
        }
        
        // Update status every 2 seconds
        setInterval(updateStatus, 2000);
        updateStatus();
    </script>
</body>
</html>
)=====";
    return html;
}

String WiFiAP::getCurrentStatus() {
    String mode = "idle";
    switch (menuSystem->getMode()) {
        case MODE_IDLE: mode = "Idle"; break;
        case MODE_SCANNING: mode = "Scanning"; break;
        case MODE_SPECTRUM: mode = "Spectrum"; break;
        case MODE_LISTENING: mode = "Listening"; break;
        case MODE_RECORDING: mode = "Recording"; break;
        case MODE_REPLAYING: mode = "Replaying"; break;
    }
    
    String json = "{";
    json += "\"mode\":\"" + mode + "\",";
    json += "\"clients\":" + String(getClientCount()) + ",";
    json += "\"ip\":\"" + getIPAddress() + "\",";
    json += "\"frequency\":" + String(menuSystem->getSelectedFrequency());
    json += "}";
    
    return json;
}
