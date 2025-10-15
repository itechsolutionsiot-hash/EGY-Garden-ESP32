#include "WebInterface.h"

WebInterface webInterface;

void WebInterface::initialize(PreferencesManager &prefs, MQTTManager &mqtt, Core &coreRef)
{
    preferences = &prefs;
    mqttManager = &mqtt;
    core = &coreRef;

    server.on("/", std::bind(&WebInterface::handleRoot, this));
    server.on("/scan", std::bind(&WebInterface::handleScan, this));
    server.on("/configure", HTTP_POST, std::bind(&WebInterface::handleConfigure, this));
    server.on("/test", HTTP_GET, std::bind(&WebInterface::handleTest, this));

    server.begin();

    Serial.println("✅ Web server started");
    Serial.println("📍 Available endpoints:");
    Serial.println("   - http://192.168.4.1/ (main page)");
    Serial.println("   - http://192.168.4.1/scan (WiFi scan)");
    Serial.println("   - http://192.168.4.1/configure (setup)");
    Serial.println("   - http://192.168.4.1/test (debug)");
}

void WebInterface::handleClient()
{
    server.handleClient();
}

void WebInterface::handleRoot()
{
    Serial.println("📄 Serving main page to client");
    server.send(200, "text/html", getMainPage());
}

void WebInterface::handleScan()
{
    Serial.println("📡 Handling WiFi scan request");
    int numNetworks = WiFi.scanNetworks();

    JsonDocument doc;
    JsonArray networks = doc["networks"].to<JsonArray>();

    for (int i = 0; i < numNetworks; i++)
    {
        networks.add(WiFi.SSID(i));
    }

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);

    Serial.println("✅ Sent " + String(numNetworks) + " networks to client");
}

void WebInterface::handleTest()
{
    Serial.println("✅ Test endpoint accessed");
    server.send(200, "text/plain", "Web server is working!");
}

void WebInterface::handleConfigure()
{
    if (server.method() == HTTP_POST)
    {
        String body = server.arg("plain");
        JsonDocument doc;
        deserializeJson(doc, body);

        String ssid = doc["ssid"];
        String password = doc["password"];
        String username = doc["username"];
        String user_password = doc["user_password"];

        Serial.println("=== CONFIGURATION RECEIVED ===");
        Serial.println("SSID: " + ssid);
        Serial.println("Username: " + username);
        Serial.println("========================");

        // Save to preferences
        preferences->setWiFiCredentials(ssid, password);
        preferences->setSystemCredentials(username, user_password);
        preferences->setConfigured(true);

        // Don't try to send credentials here - it will happen after restart
        // when WiFi and MQTT are properly connected
        Serial.println("✅ Configuration saved successfully!");
        Serial.println("🔃 Credentials will be sent to database after device restarts and connects to WiFi");

        JsonDocument responseDoc;
        responseDoc["success"] = true;
        responseDoc["message"] = "Configuration saved successfully! Device will restart and connect to your network.";

        String response;
        serializeJson(responseDoc, response);
        server.send(200, "application/json", response);

        Serial.println("🔄 Restarting device in 3 seconds...");
        delay(3000);
        ESP.restart();
    }
}
bool WebInterface::sendCredentialsToDatabase(const String &username, const String &password)
{
    bool success = mqttManager->sendCredentials(username, password);
    if (success)
    {
        credentialsSent = true;
    }
    return success;
}

String WebInterface::getMainPage()
{
    return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GreenTech - Device Setup</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        :root {
            --primary: #2d5016;
            --primary-light: #4a7c2a;
            --secondary: #4CAF50;
            --accent: #FF9800;
            --background: #f8f9fa;
            --card-bg: #ffffff;
            --text: #333333;
            --text-light: #666666;
            --border: #e1e5e9;
            --success: #28a745;
            --warning: #ffc107;
            --danger: #dc3545;
            --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1), 0 2px 4px -1px rgba(0, 0, 0, 0.06);
            --shadow-lg: 0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -2px rgba(0, 0, 0, 0.05);
        }

        body {
            font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
            line-height: 1.6;
            color: var(--text);
        }

        .setup-container {
            background: var(--card-bg);
            border-radius: 20px;
            box-shadow: var(--shadow-lg);
            overflow: hidden;
            width: 100%;
            max-width: 520px;
            animation: slideUp 0.5s ease-out;
        }

        @keyframes slideUp {
            from {
                opacity: 0;
                transform: translateY(30px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .header {
            background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
            color: white;
            padding: 30px;
            text-align: center;
            position: relative;
            overflow: hidden;
        }

        .header::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: radial-gradient(circle, rgba(255,255,255,0.1) 1px, transparent 1px);
            background-size: 20px 20px;
            animation: float 20s linear infinite;
        }

        @keyframes float {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }

        .logo {
            font-size: 3rem;
            margin-bottom: 10px;
            display: block;
        }

        .header h1 {
            font-size: 1.8rem;
            font-weight: 700;
            margin-bottom: 8px;
        }

        .header p {
            opacity: 0.9;
            font-size: 0.95rem;
        }

        .content {
            padding: 30px;
        }

        /* Progress Stepper Styles */
        .progress-stepper {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 30px;
            position: relative;
        }

        .progress-stepper::before {
            content: '';
            position: absolute;
            top: 50%;
            left: 0;
            right: 0;
            height: 3px;
            background: var(--border);
            transform: translateY(-50%);
            z-index: 1;
        }

        .progress-bar {
            position: absolute;
            top: 50%;
            left: 0;
            height: 3px;
            background: var(--secondary);
            transform: translateY(-50%);
            z-index: 2;
            transition: width 0.3s ease;
        }

        .step {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            background: white;
            border: 3px solid var(--border);
            display: flex;
            align-items: center;
            justify-content: center;
            font-weight: 600;
            color: var(--text-light);
            position: relative;
            z-index: 3;
            transition: all 0.3s ease;
        }

        .step.active {
            border-color: var(--secondary);
            background: var(--secondary);
            color: white;
            transform: scale(1.1);
        }

        .step.completed {
            border-color: var(--success);
            background: var(--success);
            color: white;
        }

        .step.completed::after {
            content: '✓';
            font-size: 1.2rem;
        }

        .step-label {
            position: absolute;
            top: 100%;
            left: 50%;
            transform: translateX(-50%);
            margin-top: 8px;
            font-size: 0.8rem;
            color: var(--text-light);
            white-space: nowrap;
            font-weight: 500;
        }

        .step.active .step-label {
            color: var(--secondary);
            font-weight: 600;
        }

        .step.completed .step-label {
            color: var(--success);
        }

        /* Step Content */
        .step-content {
            display: none;
            animation: fadeIn 0.5s ease;
        }

        .step-content.active {
            display: block;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .status-card {
            background: linear-gradient(135deg, #e3f2fd 0%, #f3e5f5 100%);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 25px;
            border-left: 4px solid var(--secondary);
        }

        .status-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 8px;
        }

        .status-item:last-child {
            margin-bottom: 0;
        }

        .status-label {
            font-weight: 600;
            color: var(--text-light);
        }

        .status-value {
            font-weight: 700;
            color: var(--primary);
        }

        .form-section {
            margin-bottom: 25px;
        }

        .section-title {
            font-size: 1.1rem;
            font-weight: 600;
            color: var(--primary);
            margin-bottom: 15px;
            display: flex;
            align-items: center;
            gap: 8px;
        }

        .section-title::before {
            content: '';
            width: 4px;
            height: 16px;
            background: var(--secondary);
            border-radius: 2px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: var(--text);
            font-size: 0.9rem;
        }

        .input-group {
            position: relative;
        }

        input, select {
            width: 100%;
            padding: 14px 16px;
            border: 2px solid var(--border);
            border-radius: 12px;
            font-size: 1rem;
            transition: all 0.3s ease;
            background: var(--card-bg);
        }

        input:focus, select:focus {
            outline: none;
            border-color: var(--secondary);
            box-shadow: 0 0 0 3px rgba(76, 175, 80, 0.1);
            transform: translateY(-2px);
        }

        .btn {
            padding: 14px 24px;
            border: none;
            border-radius: 12px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
        }

        .btn-primary {
            background: linear-gradient(135deg, var(--secondary) 0%, var(--primary-light) 100%);
            color: white;
            width: 100%;
        }

        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: var(--shadow);
        }

        .btn-primary:active {
            transform: translateY(0);
        }

        .btn-secondary {
            background: var(--background);
            color: var(--text);
            border: 2px solid var(--border);
        }

        .btn-secondary:hover {
            background: var(--border);
            transform: translateY(-1px);
        }

        .btn-next {
            background: var(--secondary);
            color: white;
        }

        .btn-prev {
            background: var(--text-light);
            color: white;
        }

        .button-group {
            display: flex;
            gap: 12px;
            margin-top: 25px;
        }

        .button-group .btn {
            flex: 1;
        }

        .networks-section {
            background: var(--background);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
        }

        .networks-header {
            display: flex;
            justify-content: between;
            align-items: center;
            margin-bottom: 15px;
        }

        .networks-list {
            max-height: 200px;
            overflow-y: auto;
            border: 2px solid var(--border);
            border-radius: 8px;
            background: var(--card-bg);
        }

        .network-item {
            padding: 12px 16px;
            border-bottom: 1px solid var(--border);
            cursor: pointer;
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .network-item:last-child {
            border-bottom: none;
        }

        .network-item:hover {
            background: var(--background);
            transform: translateX(4px);
        }

        .network-item::before {
            content: '📶';
            font-size: 1.1rem;
        }

        .alert {
            padding: 16px;
            border-radius: 12px;
            margin-bottom: 20px;
            text-align: center;
            display: none;
            animation: slideIn 0.3s ease;
            border-left: 4px solid;
        }

        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateX(-20px);
            }
            to {
                opacity: 1;
                transform: translateX(0);
            }
        }

        .alert-success {
            background: #d4edda;
            color: #155724;
            border-left-color: var(--success);
        }

        .alert-error {
            background: #f8d7da;
            color: #721c24;
            border-left-color: var(--danger);
        }

        .alert-warning {
            background: #fff3cd;
            color: #856404;
            border-left-color: var(--warning);
        }

        .loading {
            display: inline-block;
            width: 20px;
            height: 20px;
            border: 3px solid #ffffff;
            border-radius: 50%;
            border-top-color: transparent;
            animation: spin 1s ease-in-out infinite;
        }

        @keyframes spin {
            to { transform: rotate(360deg); }
        }

        .footer {
            text-align: center;
            padding: 20px;
            border-top: 1px solid var(--border);
            color: var(--text-light);
            font-size: 0.9rem;
        }

        /* Success Screen */
        .success-screen {
            text-align: center;
            padding: 40px 20px;
        }

        .success-icon {
            font-size: 4rem;
            margin-bottom: 20px;
            color: var(--success);
        }

        .success-title {
            font-size: 1.5rem;
            font-weight: 700;
            color: var(--success);
            margin-bottom: 10px;
        }

        .success-message {
            color: var(--text-light);
            margin-bottom: 30px;
        }

        .progress-animation {
            display: inline-block;
            width: 100%;
            height: 6px;
            background: var(--border);
            border-radius: 3px;
            overflow: hidden;
            margin: 20px 0;
        }

        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, var(--secondary), var(--success));
            width: 0%;
            transition: width 3s ease-in-out;
            border-radius: 3px;
        }

        /* Responsive Design */
        @media (max-width: 480px) {
            .setup-container {
                border-radius: 15px;
            }
            
            .header {
                padding: 25px 20px;
            }
            
            .content {
                padding: 25px 20px;
            }
            
            .header h1 {
                font-size: 1.5rem;
            }

            .step-label {
                font-size: 0.7rem;
            }
        }

        /* Scrollbar Styling */
        .networks-list::-webkit-scrollbar {
            width: 6px;
        }

        .networks-list::-webkit-scrollbar-track {
            background: var(--background);
            border-radius: 3px;
        }

        .networks-list::-webkit-scrollbar-thumb {
            background: var(--border);
            border-radius: 3px;
        }

        .networks-list::-webkit-scrollbar-thumb:hover {
            background: var(--text-light);
        }
    </style>
</head>
<body>
    <div class="setup-container">
        <div class="header">
            <span class="logo">🌱</span>
            <h1>GreenTech Setup</h1>
            <p>Connect your agricultural controller to WiFi</p>
        </div>

        <div class="content">
            <!-- Progress Stepper -->
            <div class="progress-stepper">
                <div class="progress-bar" id="progressBar"></div>
                <div class="step active" id="step1">
                    <span>1</span>
                    <div class="step-label">WiFi Setup</div>
                </div>
                <div class="step" id="step2">
                    <span>2</span>
                    <div class="step-label">Account</div>
                </div>
                <div class="step" id="step3">
                    <span>3</span>
                    <div class="step-label">Complete</div>
                </div>
            </div>

            <div id="alert" class="alert"></div>

            <!-- Step 1: WiFi Setup -->
            <div class="step-content active" id="stepContent1">
                <div class="status-card">
                    <div class="status-item">
                        <span class="status-label">Device ID:</span>
                        <span class="status-value" id="deviceId">Loading...</span>
                    </div>
                    <div class="status-item">
                        <span class="status-label">Status:</span>
                        <span class="status-value" id="deviceStatus">Ready to Configure</span>
                    </div>
                </div>

                <div class="form-section">
                    <div class="section-title">WiFi Configuration</div>
                    
                    <div class="networks-section">
                        <div class="networks-header">
                            <button type="button" class="btn btn-secondary" onclick="scanNetworks()" id="scanBtn">
                                <span>🔍 Scan Networks</span>
                            </button>
                        </div>
                        <div id="networks" class="networks-list">
                            <div class="network-item" style="color: var(--text-light); text-align: center;">
                                Click scan to find networks
                            </div>
                        </div>
                    </div>

                    <div class="form-group">
                        <label for="ssid">WiFi Network Name</label>
                        <div class="input-group">
                            <input type="text" id="ssid" required placeholder="Select from list or enter manually">
                        </div>
                    </div>

                    <div class="form-group">
                        <label for="password">WiFi Password</label>
                        <div class="input-group">
                            <input type="password" id="password" required placeholder="Enter WiFi password">
                        </div>
                    </div>

                    <div class="button-group">
                        <button type="button" class="btn btn-next" onclick="nextStep()">
                            Next: Account Setup →
                        </button>
                    </div>
                </div>
            </div>

            <!-- Step 2: Account Setup -->
            <div class="step-content" id="stepContent2">
                <div class="form-section">
                    <div class="section-title">System Account</div>
                    <p style="color: var(--text-light); margin-bottom: 20px; font-size: 0.9rem;">
                        Create your account to access the GreenTech dashboard and control your device remotely.
                    </p>

                    <div class="form-group">
                        <label for="username">Username</label>
                        <div class="input-group">
                            <input type="text" id="username" required placeholder="Choose your username">
                        </div>
                    </div>

                    <div class="form-group">
                        <label for="user_password">Password</label>
                        <div class="input-group">
                            <input type="password" id="user_password" required placeholder="Choose your password">
                        </div>
                    </div>

                    <div class="button-group">
                        <button type="button" class="btn btn-prev" onclick="prevStep()">
                            ← Back
                        </button>
                        <button type="button" class="btn btn-primary" onclick="saveConfiguration()" id="saveBtn">
                            <span id="btnText">Complete Setup</span>
                            <span id="btnLoading" class="loading" style="display: none;"></span>
                        </button>
                    </div>
                </div>
            </div>

            <!-- Step 3: Success Screen -->
            <div class="step-content" id="stepContent3">
                <div class="success-screen">
                    <div class="success-icon">✅</div>
                    <div class="success-title">Setup Complete!</div>
                    <div class="success-message">
                        Your GreenTech device is being configured and will restart shortly. 
                        You'll be redirected to the dashboard in a few moments.
                    </div>
                    
                    <div class="progress-animation">
                        <div class="progress-fill" id="progressFill"></div>
                    </div>
                    
                    <div style="color: var(--text-light); font-size: 0.9rem;">
                        <p>Device will automatically connect to: <strong id="connectedSSID"></strong></p>
                        <p>You can login with username: <strong id="connectedUsername"></strong></p>
                    </div>
                </div>
            </div>
        </div>

        <div class="footer">
            <p>After setup, login at: <strong>http://34.229.153.185:3000/</strong></p>
        </div>
    </div>

    <script>
        let currentStep = 1;
        const totalSteps = 3;

        // Initialize device info
        document.getElementById('deviceId').textContent = 'GT-' + Math.random().toString(36).substr(2, 8).toUpperCase();

        function updateProgressBar() {
            const progress = ((currentStep - 1) / (totalSteps - 1)) * 100;
            document.getElementById('progressBar').style.width = progress + '%';
            
            // Update step states
            for (let i = 1; i <= totalSteps; i++) {
                const step = document.getElementById('step' + i);
                const content = document.getElementById('stepContent' + i);
                
                if (i < currentStep) {
                    step.className = 'step completed';
                    content.className = 'step-content';
                } else if (i === currentStep) {
                    step.className = 'step active';
                    content.className = 'step-content active';
                } else {
                    step.className = 'step';
                    content.className = 'step-content';
                }
            }
        }

        function nextStep() {
            if (currentStep < totalSteps) {
                // Validate current step before proceeding
                if (currentStep === 1) {
                    const ssid = document.getElementById('ssid').value.trim();
                    const password = document.getElementById('password').value;
                    
                    if (!ssid || !password) {
                        showAlert('Please enter WiFi credentials', 'error');
                        return;
                    }
                }
                
                currentStep++;
                updateProgressBar();
            }
        }

        function prevStep() {
            if (currentStep > 1) {
                currentStep--;
                updateProgressBar();
            }
        }

        function showSuccessScreen(ssid, username) {
            currentStep = 3;
            updateProgressBar();
            
            document.getElementById('connectedSSID').textContent = ssid;
            document.getElementById('connectedUsername').textContent = username;
            
            // Animate progress bar
            setTimeout(() => {
                document.getElementById('progressFill').style.width = '100%';
            }, 100);
            
            // Redirect after delay
            setTimeout(() => {
                window.location.href = 'http://34.229.153.185:3000/';
            }, 5000);
        }

        async function saveConfiguration() {
            const saveBtn = document.getElementById('saveBtn');
            const btnText = document.getElementById('btnText');
            const btnLoading = document.getElementById('btnLoading');
            
            const credentials = {
                ssid: document.getElementById('ssid').value.trim(),
                password: document.getElementById('password').value,
                username: document.getElementById('username').value.trim(),
                user_password: document.getElementById('user_password').value
            };

            // Validate inputs
            if (!credentials.username || !credentials.user_password) {
                showAlert('Please fill in all account fields', 'error');
                return;
            }

            // Show loading state
            saveBtn.disabled = true;
            btnText.textContent = 'Saving...';
            btnLoading.style.display = 'inline-block';
            
            try {
                const response = await fetch('/configure', {
                    method: 'POST',
                    headers: { 
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(credentials)
                });
                
                const data = await response.json();
                
                if (data.success) {
                    showSuccessScreen(credentials.ssid, credentials.username);
                } else {
                    showAlert('❌ ' + data.message, 'error');
                    saveBtn.disabled = false;
                    btnText.textContent = 'Complete Setup';
                    btnLoading.style.display = 'none';
                }
                
            } catch (error) {
                console.error('Save error:', error);
                showAlert('❌ Network error. Please try again.', 'error');
                saveBtn.disabled = false;
                btnText.textContent = 'Complete Setup';
                btnLoading.style.display = 'none';
            }
        }

        // Existing functions (keep these from your original code)
        async function scanNetworks() {
            const scanBtn = document.getElementById('scanBtn');
            const networksDiv = document.getElementById('networks');
            
            scanBtn.disabled = true;
            scanBtn.innerHTML = '<span class="loading"></span> Scanning...';
            
            try {
                const response = await fetch('/scan');
                const data = await response.json();
                
                networksDiv.innerHTML = '';
                
                if (data.networks && data.networks.length > 0) {
                    data.networks.forEach(network => {
                        const div = document.createElement('div');
                        div.className = 'network-item';
                        div.innerHTML = `<span>${network}</span>`;
                        div.onclick = () => {
                            document.getElementById('ssid').value = network;
                            document.querySelectorAll('.network-item').forEach(item => {
                                item.style.background = '';
                            });
                            div.style.background = 'var(--background)';
                        };
                        networksDiv.appendChild(div);
                    });
                } else {
                    networksDiv.innerHTML = '<div class="network-item" style="color: var(--text-light); text-align: center;">No networks found</div>';
                }
            } catch (error) {
                networksDiv.innerHTML = '<div class="network-item" style="color: var(--danger); text-align: center;">Scan failed</div>';
                console.error('Scan error:', error);
            } finally {
                scanBtn.disabled = false;
                scanBtn.innerHTML = '<span>🔍 Scan Networks</span>';
            }
        }

        function showAlert(message, type) {
            const alert = document.getElementById('alert');
            alert.textContent = message;
            alert.className = `alert alert-${type}`;
            alert.style.display = 'block';
            
            if (type === 'success') {
                setTimeout(() => {
                    alert.style.display = 'none';
                }, 5000);
            }
        }

        // Auto-scan on page load
        window.addEventListener('load', () => {
            setTimeout(scanNetworks, 1000);
            updateProgressBar();
        });

        // Input validation
        document.getElementById('ssid').addEventListener('input', validateInput);
        document.getElementById('password').addEventListener('input', validateInput);
        document.getElementById('username').addEventListener('input', validateInput);
        document.getElementById('user_password').addEventListener('input', validateInput);

        function validateInput(e) {
            const input = e.target;
            if (input.value.trim()) {
                input.style.borderColor = 'var(--secondary)';
            } else {
                input.style.borderColor = 'var(--border)';
            }
        }
    </script>
</body>
</html>
)rawliteral";
}