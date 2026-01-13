#include "Hardware.h"
#include "Config.h"
#include "WifiConfig.h"
#include "MqttHandler.h"
#include "Detector.h"
#include "neotimer.h"
#include "Arduino.h"

#ifdef SERIAL_DEBUG_BENCH
    constexpr int TIMER_BENCH_MS = 500;
#endif

void setup() {
    Hardware::OnButtonPressed = WifiConfig::HandleButtonPressed;
    Config::OnConfigChange = MqttHandler::HandleConfigChange;
    Config::OnAddParameter = WifiConfig::HandleAddParameter;
    WifiConfig::OnParamsSave = Config::HandleParamsSave;
    MqttHandler::OnConfigReceived = Detector::HandleConfigReceived;
    Detector::OnDetection = MqttHandler::HandleDetection;

    Hardware::Setup();
    Config::Setup();
    WifiConfig::Setup();
    MqttHandler::Setup();
    Detector::Setup();
}

void loop() {
    #ifdef SERIAL_DEBUG_BENCH
        unsigned long t0 = micros();
    #endif

    Hardware::Loop();

    #ifdef SERIAL_DEBUG_BENCH
        unsigned long t1 = micros();
        unsigned long t2 = t1, t3 = t1;
    #endif

    WifiConfig::Loop();

    #ifdef SERIAL_DEBUG_BENCH
        t2 = micros();
    #endif

    if (WifiConfig::GetIsWiFiConnected()) {
        MqttHandler::Loop();

        #ifdef SERIAL_DEBUG_BENCH
            t3 = micros();
        #endif
    }

    #ifdef SERIAL_DEBUG_BENCH
        static Neotimer benchTimer(TIMER_BENCH_MS);
        if (!benchTimer.waiting()) {
            Hardware::Log("HW: %luus | Wifi: %luus | MQTT: %luus | Total: %luus", 
                (t1 - t0), 
                (t2 - t1), 
                (t3 - t2), 
                (micros() - t0)
            );
            benchTimer.start();
        }
    #endif
}
