#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>

class PreferencesManager
{
public:
    void initialize();
    void end();

    // WiFi preferences
    String getWiFiSSID() { return preferences.getString("wifi_ssid", ""); }
    String getWiFiPassword() { return preferences.getString("wifi_pass", ""); }
    void setWiFiCredentials(const String &ssid, const String &password);

    // System credentials
    String getSystemUsername() { return preferences.getString("sys_username", ""); }
    String getSystemPassword() { return preferences.getString("sys_password", ""); }
    void setSystemCredentials(const String &username, const String &password);

    // Configuration status
    bool isConfigured() { return preferences.getBool("configured", false); }
    void setConfigured(bool configured) { preferences.putBool("configured", configured); }

    // Get Preferences reference for other modules
    Preferences &getPreferences() { return preferences; }

private:
    Preferences preferences;
};

extern PreferencesManager preferencesManager; // Declaration only

#endif