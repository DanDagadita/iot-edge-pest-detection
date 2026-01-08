#include <Arduino.h>

// Switch mode here:
//#define MODE_COLLECT  
#define MODE_DETECT 

constexpr int SENSOR_PIN = 32;
constexpr int INDICATOR_LED_PIN = 2;
constexpr int SAMPLING_WINDOW_MS = 10;

#ifdef MODE_COLLECT
    #include "Collector.h"
#endif

#ifdef MODE_DETECT
    #include "Config.h"
    #include "MqttHandler.h"
    #include "Detector.h"
#endif

void setup() {
    Serial.begin(115200);
    pinMode(SENSOR_PIN, INPUT);
    pinMode(INDICATOR_LED_PIN, OUTPUT);

    #ifdef MODE_COLLECT
        Collector::setup();
    #endif

    #ifdef MODE_DETECT
        Config::setup();
        MqttHandler::setup();
        Detector::setup();

        Detector::onDetection = MqttHandler::handleDetection;
        MqttHandler::onCommandReceived = Detector::handleRemoteConfig;
        Config::onConfigChange = MqttHandler::syncWithConfig;
    #endif
}

void loop() {
    #ifdef MODE_COLLECT
        Collector::loop(SENSOR_PIN, INDICATOR_LED_PIN, SAMPLING_WINDOW_MS);
    #endif

    #ifdef MODE_DETECT
        Config::loop();

        if (Config::isWifiConnected) {
            MqttHandler::loop();

            if (MqttHandler::isMqttConnected) {
                Detector::loop(SENSOR_PIN, INDICATOR_LED_PIN, SAMPLING_WINDOW_MS);
            }
        }
    #endif
}
