#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "Hardware.h"

namespace Collector {
    void setup() {
        Hardware::log("timestamp_ms,intensity");
    }

    void loop() {
        static int activeSignalTicks = 0;
        static int totalSignalTicks = 0;

        if (Hardware::getMicrophoneState()) {
            activeSignalTicks++;
        }

        totalSignalTicks++;

        unsigned long now = millis();
        static unsigned long lastLoop = 0;
        if (lastLoop == 0 || now - lastLoop > Hardware::SAMPLING_WINDOW_MS) {
            float intensity = (float)activeSignalTicks / totalSignalTicks * 100.0;

            Hardware::log("%lu,%.2f", now, intensity);
            Hardware::setIndicator((int)(intensity * 2.55), false);

            activeSignalTicks = 0;
            totalSignalTicks = 0;
            lastLoop = millis();
        }
    }
}

#endif
