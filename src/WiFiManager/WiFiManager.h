#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "PreferencesManager/PreferencesManager.h"

class WiFiManager
{
public:
    void initialize(PreferencesManager &prefs);
    void startSoftAP();
    void connectToWiFi();
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    String getIPAddress() const { return WiFi.localIP().toString(); }

private:
    PreferencesManager *preferences;
    const char *SOFT_AP_SSID = "green-tech";
    const char *SOFT_AP_PASSWORD = "12345678";
};

extern WiFiManager wifiManager; // Declaration only

#endif