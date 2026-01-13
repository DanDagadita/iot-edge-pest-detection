#ifndef DETECTOR_H
#define DETECTOR_H

#include <ArduinoJson.h>

namespace Detector {
    extern void (*OnDetection)(const float, const float);

    void HandleConfigReceived(const JsonDocument& doc);

    void Setup();
}

#endif
