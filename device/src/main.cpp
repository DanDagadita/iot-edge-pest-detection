#include "Hardware.h"

// Switch mode here:
//#define MODE_COLLECT  
#define MODE_DETECT 

#ifdef MODE_COLLECT
    #include "Collector.h"
#endif

#ifdef MODE_DETECT
    #include "Config.h"
    #include "WifiConfig.h"
    #include "MqttHandler.h"
    #include "Detector.h"
#endif

//#define SERIAL_DEBUG_BENCH

#include "Arduino.h"

constexpr int LOOP_LIMIT_BENCH_MS = 500;

void setup() {
    #ifdef MODE_COLLECT
        Hardware::Setup();
        Collector::Setup();
    #endif

    #ifdef MODE_DETECT
        Hardware::OnButtonPressed = WifiConfig::HandleButtonPressed;
        Hardware::Setup();
        Config::OnConfigChange = MqttHandler::HandleConfigChange;
        Config::OnAddParameter = WifiConfig::HandleAddParameter;
        WifiConfig::OnParamsSave = Config::HandleParamsSave;
        MqttHandler::OnConfigReceived = Detector::HandleConfigReceived;
        Detector::OnDetection = MqttHandler::HandleDetection;
        Config::Setup();
        WifiConfig::Setup();
        MqttHandler::Setup();
        Detector::Setup();
    #endif
}

void loop() {
    unsigned long t0 = micros();
    Hardware::Loop();
    unsigned long t1 = micros();

    #ifdef MODE_DETECT
        unsigned long t2 = t1, t3 = t1;
        WifiConfig::Loop();
        t2 = micros();

        if (WifiConfig::GetIsWiFiConnected()) {
            MqttHandler::Loop();
            t3 = micros();
        }

        #ifdef SERIAL_DEBUG_BENCH
            static unsigned long lastLoop = 0;
            if (millis() - lastLoop > LOOP_LIMIT_BENCH_MS) {
                Serial.printf("HW: %luus | Wifi: %luus | MQTT: %luus | Total: %luus\n", 
                    (t1 - t0), 
                    (t2 - t1), 
                    (t3 - t2), 
                    (micros() - t0)
                );
                lastLoop = millis();
            }
        #endif
    #endif
}
