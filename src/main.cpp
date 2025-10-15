#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Preferences.h>

// Include all module headers
#include "Core/Core.h"
#include "WiFiManager/WiFiManager.h"
#include "MQTTManager/MQTTManager.h"
#include "RelayController/RelayController.h"
#include "WebInterface/WebInterface.h"
#include "PreferencesManager/PreferencesManager.h"

// Only declare WiFiClient here - all other globals are defined in their respective .cpp files
WiFiClient wifiClient;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.println("📨 MQTT Received: " + message);

    JsonDocument doc;
    deserializeJson(doc, message);

    String targetDevice = doc["deviceId"];
    if (targetDevice != core.getDeviceId())
        return;

    int relayIndex = doc["relay"];
    String action = doc["action"];

    if (action == "on")
    {
        relayController.setRelayState(relayIndex, true);
    }
    else if (action == "off")
    {
        relayController.setRelayState(relayIndex, false);
    }
    else if (action == "toggle")
    {
        relayController.setRelayState(relayIndex, !relayController.getRelayState(relayIndex));
    }
    else if (action == "timer")
    {
        unsigned long duration = doc["duration"];
        relayController.setRelayTimer(relayIndex, duration);
    }

    // Send status update
    mqttManager.sendRelayStatus(relayIndex,
                                relayController.getRelayState(relayIndex),
                                relayController.getRelayTimer(relayIndex));
}

void setup()
{
    core.initialize();
    preferencesManager.initialize();
    relayController.initialize();

    core.setDeviceConfigured(preferencesManager.isConfigured());

    // Initialize WiFi Manager with preferences
    wifiManager.initialize(preferencesManager);

    if (!core.isDeviceConfigured())
    {
        Serial.println("🚀 Starting Setup Mode...");
        wifiManager.startSoftAP();
        webInterface.initialize(preferencesManager, mqttManager, core);
        Serial.println("📍 Setup Portal Ready at: http://192.168.4.1");
        Serial.println("📶 Connect to WiFi: green-tech");
        Serial.println("🔑 Password: 12345678");
    }
    else
    {
        Serial.println("🔗 Connecting to WiFi...");
        wifiManager.connectToWiFi();
    }

    // Initialize MQTT (but don't connect immediately)
    mqttManager.initialize(wifiClient, core);
    mqttManager.setCallback(mqttCallback);

    Serial.println("==========================================");
}

void checkAndSendCredentials()
{
    String username = preferencesManager.getSystemUsername();
    String password = preferencesManager.getSystemPassword();

    if (username != "" && password != "" && !webInterface.areCredentialsSent())
    {
        Serial.println("🔄 Found unsent credentials. Attempting to send to database...");
        if (mqttManager.sendCredentials(username, password))
        {
            Serial.println("✅ Previously saved credentials sent to database successfully!");
            webInterface.setCredentialsSent(true);
        }
        else
        {
            Serial.println("❌ Failed to send credentials. Will retry later...");
        }
    }
}

void loop()
{
    if (!core.isDeviceConfigured())
    {
        // In setup mode, handle web server clients
        webInterface.handleClient();
        delay(10);
    }
    else
    {
        // In normal operation mode
        if (wifiManager.isConnected())
        {
            // Ensure MQTT is connected
            if (!mqttManager.isConnected())
            {
                Serial.println("🔗 Attempting MQTT connection...");
                if (mqttManager.connect())
                {
                    Serial.println("✅ MQTT connected!");

                    // Try to send credentials immediately after MQTT connection
                    static bool credentialsChecked = false;
                    if (!credentialsChecked)
                    {
                        checkAndSendCredentials();
                        credentialsChecked = true;
                    }
                }
                else
                {
                    Serial.println("❌ MQTT connection failed, will retry");
                }
            }

            // Handle MQTT messages
            mqttManager.loop();
            relayController.checkRelayTimers();

            // Send device status periodically
            static unsigned long lastStatusUpdate = 0;
            if (millis() - lastStatusUpdate > 30000)
            { // Every 30 seconds
                if (mqttManager.isConnected())
                {
                    mqttManager.sendDeviceStatus(core.getDeviceId(),
                                                 relayController.getRelayStates(),
                                                 relayController.getRelayTimers(),
                                                 relayController.RELAY_COUNT);
                    lastStatusUpdate = millis();
                    Serial.println("📊 Device status sent to MQTT");
                }
            }

            // Periodically check if we need to send credentials (if not already sent)
            static unsigned long lastCredentialCheck = 0;
            if (millis() - lastCredentialCheck > 60000 && !webInterface.areCredentialsSent())
            {
                checkAndSendCredentials();
                lastCredentialCheck = millis();
            }
        }
        else
        {
            Serial.println("🚀 Starting Setup Mode...");
            wifiManager.startSoftAP();
            webInterface.initialize(preferencesManager, mqttManager, core);
            Serial.println("📍 Setup Portal Ready at: http://192.168.4.1");
            Serial.println("📶 Connect to WiFi: green-tech");
            Serial.println("🔑 Password: 12345678");
            // WiFi not connected, try to reconnect
            Serial.println("📡 WiFi disconnected, attempting to reconnect...");
            wifiManager.connectToWiFi();
        }
    }
}