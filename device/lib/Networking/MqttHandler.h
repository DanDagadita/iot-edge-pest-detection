#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <ArduinoJson.h>

namespace MqttHandler {
    extern void (*OnConfigReceived)(const JsonDocument&);

    bool GetIsMqttConnected();

    void HandleDetection(const float probability, const float intensity);
    void HandleConfigChange();

    void Setup();
    void Loop();
}

#endif
