#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// AP credentials
static const char* ssid = "REMOTE_LAUNCHPAD";
static const char* password = "12345678";

// Built-in LED pin
const int ledPin = LED_BUILTIN;

// Relay (in signal): D1 (GPIO5)
const int relayPin = 5;
const bool RELAY_ACTIVE_LOW = HIGH;

// Soft switch : D2 (GPIO4)
const int softSwitchPin = 4; // D2 en NodeMCU

// Buzzer : D5 (GPIO14)
const int buzzerPin = 14;

#endif // CONFIG_H