#include <Arduino.h>

// Switch mode here:
//#define MODE_COLLECT  
#define MODE_DETECT 

constexpr int DIGITAL_PIN = 32;
constexpr int LED_PIN = 2;
constexpr int WINDOW_MS = 10;

#ifdef MODE_COLLECT
    #include "Collector.h"
#endif

#ifdef MODE_DETECT
    #include "Config.h"
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
        Config::setup();
        Detector::setup();
    #endif
}

void loop() {
    #ifdef MODE_COLLECT
        Collector::run(DIGITAL_PIN, LED_PIN, WINDOW_MS);
    #endif

    #ifdef MODE_DETECT
        Config::loop();
        if (Config::connection_success) {
            Detector::run(DIGITAL_PIN, LED_PIN, WINDOW_MS);
        }
    #endif
}
