#include "WiFiManager.h"

WiFiManager wifiManager;

void WiFiManager::initialize(PreferencesManager &prefs)
{
    preferences = &prefs;
}

void WiFiManager::startSoftAP()
{
    WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD);
    Serial.println("Soft AP Started");
    Serial.print("SSID: ");
    Serial.println(SOFT_AP_SSID);
    Serial.print("Password: ");
    Serial.println(SOFT_AP_PASSWORD);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void WiFiManager::connectToWiFi()
{
    String ssid = preferences->getWiFiSSID();
    String password = preferences->getWiFiPassword();

    if (ssid == "")
    {
        Serial.println("❌ No WiFi credentials found");
        return;
    }

    Serial.print("📡 Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✅ WiFi Connected!");
        Serial.print("📍 IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\n❌ WiFi Connection Failed");
    }
}