#ifndef CORE_H
#define CORE_H

#include <Arduino.h>

class Core
{
public:
    void initialize();
    String getDeviceId() const { return deviceId; }
    unsigned long getUptime() const { return millis() - deviceStartTime; }
    bool isDeviceConfigured() const { return isConfigured; }
    void setDeviceConfigured(bool configured) { isConfigured = configured; }

private:
    void generateDeviceId();

    String deviceId;
    unsigned long deviceStartTime = 0;
    bool isConfigured = false;
};

extern Core core; // Declaration only

#endif