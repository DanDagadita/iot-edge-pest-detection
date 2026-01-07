#include <Arduino.h>

// Switch mode here:
// Comment out the one you don't want to use.
//#define MODE_COLLECT  // Use this to record CSV data for training
#define MODE_DETECT // Use this to run the AI and detect pests

constexpr int DIGITAL_PIN = 32;
constexpr int LED_PIN = 2;
constexpr int WINDOW_MS = 10;

#ifdef MODE_COLLECT
    #include "Collector.h"
#endif

#ifdef MODE_DETECT
    #include "Detector.h"
#endif

void setup() {
    Serial.begin(115200);
    pinMode(DIGITAL_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    #ifdef MODE_COLLECT
        Collector::setup();
    #endif

    #ifdef MODE_DETECT
        Detector::setup();
    #endif
}

void loop() {
    #ifdef MODE_COLLECT
        Collector::run(DIGITAL_PIN, LED_PIN, WINDOW_MS);
    #endif

    #ifdef MODE_DETECT
        Detector::run(DIGITAL_PIN, LED_PIN, WINDOW_MS);
    #endif
}
