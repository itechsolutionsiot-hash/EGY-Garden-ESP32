#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

class RelayController
{
public:
    void initialize();
    void setRelayState(int relayIndex, bool state);
    void setRelayTimer(int relayIndex, unsigned long duration);
    void checkRelayTimers();
    bool getRelayState(int relayIndex) const;
    unsigned long getRelayTimer(int relayIndex) const;

    // Provide access to arrays for MQTT
    const bool *getRelayStates() const { return relayStates; }
    const unsigned long *getRelayTimers() const { return relayTimers; }

    static const int RELAY_COUNT = 20;

private:
    const int RELAY_PINS[RELAY_COUNT] = {
        2, 4, 5, 12, 13, 14, 15, 16, 17, 18,
        19, 21, 22, 23, 25, 26, 27, 32, 33, 35}; // Changed 34 to 35

    bool relayStates[RELAY_COUNT] = {false};
    unsigned long relayTimers[RELAY_COUNT] = {0};
};

extern RelayController relayController; // Declaration only

#endif