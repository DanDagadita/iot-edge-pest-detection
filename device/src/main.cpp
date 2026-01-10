#include "Hardware.h"

// Switch mode here:
//#define MODE_COLLECT  
#define MODE_DETECT 

#ifdef MODE_COLLECT
    #include "Collector.h"
#endif

#ifdef MODE_DETECT
    #include "Config.h"
    #include "MqttHandler.h"
    #include "Detector.h"
#endif

void setup() {
    Hardware::setup();

    #ifdef MODE_COLLECT
        Collector::setup();
    #endif

    #ifdef MODE_DETECT
        Config::setup();
        MqttHandler::setup();
        Detector::setup();

        Detector::onDetection = MqttHandler::handleDetection;
        MqttHandler::onConfigReceived = Detector::handleConfigReceived;
        Config::onConfigChange = MqttHandler::handleConfigChange;
    #endif
}

void loop() {
    #ifdef MODE_COLLECT
        unsigned long t0 = micros();
        Collector::loop();
        unsigned long t1 = micros();

        //Serial.printf("Collector: %luus\n", (t1 - t0));
    #endif

    #ifdef MODE_DETECT
        unsigned long t0 = micros();
        Config::loop();
        unsigned long t1 = micros();
        
        if (Config::isWifiConnected) {
            MqttHandler::loop();
        }
        unsigned long t2 = micros();

        if (MqttHandler::isMqttConnected) {
            Detector::loop();
        }
        unsigned long t3 = micros();

        //Serial.printf("Config: %luus | MQTT: %luus | Detector: %luus\n", (t1 - t0), (t2 - t1), (t3 - t2));
    #endif
}
