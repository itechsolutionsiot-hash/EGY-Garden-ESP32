#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "Core/Core.h"

class MQTTManager
{
public:
    void initialize(WiFiClient &client, Core &coreRef);
    bool connect();
    void loop();
    void setCallback(void (*callback)(char *, byte *, unsigned int));
    bool publish(const char *topic, const char *message);
    bool isConnected() { return mqttClient.connected(); }

    // Specific message methods
    bool sendCredentials(const String &username, const String &password);
    void sendRelayStatus(int relayIndex, bool state, unsigned long timer);
    void sendDeviceStatus(const String &deviceId, const bool *relayStates,
                          const unsigned long *relayTimers, int relayCount);

private:
    PubSubClient mqttClient;
    Core *core;
    const char *MQTT_SERVER = "34.229.153.185";
    const int MQTT_PORT = 1883;
    const char *MQTT_TOPIC_CREDENTIALS = "green-tech/credentials";
    const char *MQTT_TOPIC_RELAY_CONTROL = "green-tech/relay-control";
    const char *MQTT_TOPIC_RELAY_STATUS = "green-tech/relay-status";
    const char *MQTT_TOPIC_DEVICE_STATUS = "green-tech/device-status";
};

extern MQTTManager mqttManager; // Declaration only

#endif