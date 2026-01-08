#ifndef COLLECTOR_H
#define COLLECTOR_H

#include <Arduino.h>

namespace Collector {
    unsigned long last_window_time = 0;
    int high_count = 0;
    int total_count = 0;

    void setup() {
        Serial.println("timestamp_ms,intensity");
    }

    void loop(int digitalPin, int ledPin, int windowMs) {
        if (digitalRead(digitalPin) == HIGH) {
            high_count++;
        }

        total_count++;

        unsigned long now = millis();
        if (now - last_window_time >= windowMs) {
            float intensity = (float)high_count / total_count * 100.0;

            Serial.print(now);
            Serial.print(",");
            Serial.println(intensity, 2);

            analogWrite(ledPin, (int)(intensity * 2.55));

            high_count = 0;
            total_count = 0;
            last_window_time = now;
        }
    }
}

#endif
