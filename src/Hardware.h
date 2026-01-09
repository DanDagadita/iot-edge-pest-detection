#include <Arduino.h>

#ifndef HARDWARE_H
#define HARDWARE_H

namespace Hardware {
    constexpr int PIN_SENSOR = 32;
    constexpr int PIN_LED = 2;
    constexpr int SAMPLING_WINDOW_MS = 10;

    void setup() {
        Serial.begin(115200);
        pinMode(PIN_SENSOR, INPUT);
        pinMode(PIN_LED, OUTPUT);
    }

    template<typename... Args>
    void log(const char* format, Args... args) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), format, args...);
        Serial.println(buffer);
    }

    bool getMicrophoneState() {
        return digitalRead(PIN_SENSOR) == HIGH;
    }

    void setIndicator(int value, bool isDigital) {
        if (isDigital) {
            digitalWrite(PIN_LED, HIGH);
        } else {
            analogWrite(PIN_LED, value);
        }
    }
}

#endif
