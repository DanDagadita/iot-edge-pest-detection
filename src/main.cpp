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
        MqttHandler::onCommandReceived = Detector::handleCommandReceived;
        Config::onConfigChange = MqttHandler::handleConfigChange;
    #endif
}

void loop() {
    #ifdef MODE_COLLECT
        Collector::loop();
    #endif

    #ifdef MODE_DETECT
        Config::loop();

        if (Config::isWifiConnected) {
            MqttHandler::loop();

            if (MqttHandler::isMqttConnected) {
                Detector::loop();
            }
        }
    #endif
}
