#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "Hardware.h"

namespace Collector {
    unsigned long lastWindowTime = 0;
    int highCount = 0;
    int totalCount = 0;

    void setup() {
        Hardware::log("timestamp_ms,intensity");
    }

    void loop() {
        if (Hardware::getMicrophoneState()) {
            highCount++;
        }

        totalCount++;

        unsigned long now = millis();
        if (now - lastWindowTime >= Hardware::SAMPLING_WINDOW_MS) {
            float intensity = (float)highCount / totalCount * 100.0;

            Hardware::log("%lu,%.2f", now, intensity);
            Hardware::setIndicator((int)(intensity * 2.55), false);

            highCount = 0;
            totalCount = 0;
            lastWindowTime = now;
        }
    }
}

#endif
