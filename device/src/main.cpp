#include "Hardware.h"

// Switch mode here:
//#define MODE_COLLECT  
#define MODE_DETECT 

#ifdef MODE_COLLECT
    #include "Collector.h"
#endif

#ifdef MODE_DETECT
    #include "Config.h"
    #include "WiFiConfig.h"
    #include "MqttHandler.h"
    #include "Detector.h"
#endif

void setup() {
    Hardware::onButtonPressed = WiFiConfig::handleButtonPressed;
    Hardware::setup();

    #ifdef MODE_COLLECT
        Collector::setup();
    #endif

    #ifdef MODE_DETECT
        Config::onConfigChange = MqttHandler::handleConfigChange;
        Config::onAddParameter = WiFiConfig::handleAddParameter;
        WiFiConfig::onParamsSave = Config::handleParamsSave;
        MqttHandler::onConfigReceived = Detector::handleConfigReceived;
        Detector::onDetection = MqttHandler::handleDetection;
        Config::setup();
        WiFiConfig::setup();
        MqttHandler::setup();
        Detector::setup();
    #endif
}

void loop() {
    Hardware::loop();

    #ifdef MODE_COLLECT
        unsigned long t0 = micros();
        Collector::loop();
        unsigned long t1 = micros();

        //Serial.printf("Collector: %luus\n", (t1 - t0));
    #endif

    #ifdef MODE_DETECT
        unsigned long t0 = micros();
        WiFiConfig::loop();
        unsigned long t1, t2, t3 = micros();

        if (WiFiConfig::isWifiConnected) {
            MqttHandler::loop();
            t2 = micros();

            if (MqttHandler::isMqttConnected) {
                Detector::loop();
            }
            t3 = micros();
        }

        //Serial.printf("Config: %luus | MQTT: %luus | Detector: %luus\n", (t1 - t0), (t2 - t1), (t3 - t2));
    #endif
}
