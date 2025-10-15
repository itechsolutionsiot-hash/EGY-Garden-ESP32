#include "PreferencesManager.h"

PreferencesManager preferencesManager;

void PreferencesManager::initialize()
{
    preferences.begin("green-tech", false);
}

void PreferencesManager::end()
{
    preferences.end();
}

void PreferencesManager::setWiFiCredentials(const String &ssid, const String &password)
{
    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_pass", password);
}

void PreferencesManager::setSystemCredentials(const String &username, const String &password)
{
    preferences.putString("sys_username", username);
    preferences.putString("sys_password", password);
}