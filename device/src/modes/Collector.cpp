#include "Collector.h"
#include "Hardware.h"
#include "neotimer.h"
#include <Arduino.h>

void Collector::Setup() {
    Hardware::Log("timestamp_ms,intensity");
}

void Collector::Loop() {
    static int activeSignalTicks = 0;
    static int totalSignalTicks = 0;

    if (Hardware::GetMicrophoneState()) {
        activeSignalTicks++;
    }

    totalSignalTicks++;

    static Neotimer loopTimer(Hardware::SAMPLING_WINDOW_MS);
    if (!loopTimer.waiting()) {
        float intensity = (float)activeSignalTicks / totalSignalTicks * 100.0;

        Hardware::Log("%lu,%.2f", millis(), intensity);
        Hardware::SetIndicator((int)(intensity * 2.55), false);

        activeSignalTicks = 0;
        totalSignalTicks = 0;
        loopTimer.start();
    }
}
