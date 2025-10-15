#include "RelayController.h"

RelayController relayController;

void RelayController::initialize()
{
    for (int i = 0; i < RELAY_COUNT; i++)
    {
        pinMode(RELAY_PINS[i], OUTPUT);
        digitalWrite(RELAY_PINS[i], LOW);
        Serial.println("Initialized Relay " + String(i) + " on pin " + String(RELAY_PINS[i]));
    }
}

void RelayController::setRelayState(int relayIndex, bool state)
{
    if (relayIndex >= 0 && relayIndex < RELAY_COUNT)
    {
        relayStates[relayIndex] = state;
        digitalWrite(RELAY_PINS[relayIndex], state ? HIGH : LOW);
        Serial.println("🔌 Relay " + String(relayIndex) + " → " + (state ? "ON" : "OFF"));
    }
}

void RelayController::setRelayTimer(int relayIndex, unsigned long duration)
{
    if (relayIndex >= 0 && relayIndex < RELAY_COUNT)
    {
        setRelayState(relayIndex, true);
        relayTimers[relayIndex] = millis() + (duration * 1000);
        Serial.println("⏰ Relay " + String(relayIndex) + " timer: " + String(duration) + "s");
    }
}

void RelayController::checkRelayTimers()
{
    unsigned long currentTime = millis();
    for (int i = 0; i < RELAY_COUNT; i++)
    {
        if (relayTimers[i] > 0 && currentTime >= relayTimers[i])
        {
            setRelayState(i, false);
            relayTimers[i] = 0;
        }
    }
}

bool RelayController::getRelayState(int relayIndex) const
{
    return (relayIndex >= 0 && relayIndex < RELAY_COUNT) ? relayStates[relayIndex] : false;
}

unsigned long RelayController::getRelayTimer(int relayIndex) const
{
    return (relayIndex >= 0 && relayIndex < RELAY_COUNT) ? relayTimers[relayIndex] : 0;
}