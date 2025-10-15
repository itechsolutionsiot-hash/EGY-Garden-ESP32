#include "MQTTManager.h"

MQTTManager mqttManager;

void MQTTManager::initialize(WiFiClient &client, Core &coreRef)
{
    mqttClient.setClient(client);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    core = &coreRef;
}

bool MQTTManager::connect()
{
    if (mqttClient.connected())
        return true;

    Serial.print("🔗 Connecting to MQTT...");
    mqttClient.setSocketTimeout(10);

    // Add a small delay to ensure network is ready
    delay(1000);

    String deviceId = core->getDeviceId();
    if (mqttClient.connect(deviceId.c_str()))
    {
        Serial.println("✅ Connected!");
        mqttClient.subscribe(MQTT_TOPIC_RELAY_CONTROL);
        Serial.println("📡 Subscribed to relay control topic: " + String(MQTT_TOPIC_RELAY_CONTROL));
        return true;
    }
    else
    {
        Serial.println("❌ Failed! Error: " + String(mqttClient.state()));
        return false;
    }
}

void MQTTManager::loop()
{
    mqttClient.loop();
}

void MQTTManager::setCallback(void (*callback)(char *, byte *, unsigned int))
{
    mqttClient.setCallback(callback);
}

bool MQTTManager::publish(const char *topic, const char *message)
{
    return mqttClient.publish(topic, message);
}

bool MQTTManager::sendCredentials(const String &username, const String &password)
{
    Serial.println("🔄 Attempting to send credentials to database via MQTT...");

    if (!connect())
    {
        Serial.println("❌ Cannot send credentials - MQTT not connected");
        return false;
    }

    JsonDocument doc;
    doc["deviceId"] = core->getDeviceId();
    doc["username"] = username;
    doc["password"] = password;
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);

    Serial.println("📤 Sending credentials to MQTT topic: " + String(MQTT_TOPIC_CREDENTIALS));
    Serial.println("📝 Message: " + message);

    bool success = publish(MQTT_TOPIC_CREDENTIALS, message.c_str());

    if (success)
    {
        Serial.println("✅ Credentials sent to database via MQTT successfully!");
    }
    else
    {
        Serial.println("❌ Failed to send credentials via MQTT!");
    }

    return success;
}

void MQTTManager::sendRelayStatus(int relayIndex, bool state, unsigned long timer)
{
    if (!isConnected())
        return;

    JsonDocument doc;
    doc["deviceId"] = core->getDeviceId();
    doc["relay"] = relayIndex;
    doc["state"] = state;
    doc["timer"] = timer;
    doc["timestamp"] = millis();

    String message;
    serializeJson(doc, message);
    publish(MQTT_TOPIC_RELAY_STATUS, message.c_str());
}

void MQTTManager::sendDeviceStatus(const String &deviceId, const bool *relayStates,
                                   const unsigned long *relayTimers, int relayCount)
{
    if (!isConnected())
        return;

    JsonDocument doc;
    doc["deviceId"] = deviceId;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi"] = WiFi.RSSI();
    doc["uptime"] = core->getUptime();
    doc["timestamp"] = millis();

    JsonArray relays = doc["relays"].to<JsonArray>();
    for (int i = 0; i < relayCount; i++)
    {
        JsonObject relay = relays.add<JsonObject>();
        relay["index"] = i;
        relay["state"] = relayStates[i];
        relay["timer"] = relayTimers[i];
    }

    String message;
    serializeJson(doc, message);
    publish(MQTT_TOPIC_DEVICE_STATUS, message.c_str());
}