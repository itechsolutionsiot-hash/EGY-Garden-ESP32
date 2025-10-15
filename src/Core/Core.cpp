#include "Core.h"

Core core;

void Core::initialize()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n🌱 GreenTech Relay Controller Starting...");
    Serial.println("==========================================");

    generateDeviceId();
    deviceStartTime = millis();
}

void Core::generateDeviceId()
{
    deviceId = "GT-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.print("📟 Device ID: ");
    Serial.println(deviceId);
}