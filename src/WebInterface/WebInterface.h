#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "PreferencesManager/PreferencesManager.h"
#include "MQTTManager/MQTTManager.h"
#include "Core/Core.h"

class WebInterface
{
public:
    void initialize(PreferencesManager &prefs, MQTTManager &mqtt, Core &coreRef);
    void handleClient();
    void handleRoot();
    void handleScan();
    void handleConfigure();
    void handleTest(); // Add this line
    String getMainPage();
    bool areCredentialsSent() const { return credentialsSent; }
    void setCredentialsSent(bool sent) { credentialsSent = sent; }

private:
    WebServer server{80};
    PreferencesManager *preferences;
    MQTTManager *mqttManager;
    Core *core;
    bool credentialsSent = false;

    bool sendCredentialsToDatabase(const String &username, const String &password);
};

extern WebInterface webInterface;

#endif