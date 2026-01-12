#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

namespace Hardware {
    constexpr int SAMPLING_WINDOW_MS = 10;

    extern void (*OnButtonPressed)();

    void PrintToLCD(const char* format, ...);
    void Log(const char* format, ...);

    uint16_t ReadMicrophone();
    void SetIndicator(const int value, const bool isDigital);

    void Setup();
    void Loop();
}

#endif
