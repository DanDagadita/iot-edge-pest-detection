#include "Collector.h"
#include "Hardware.h"
#include "arduinoFFT.h"
#include <Arduino.h>

namespace {
    constexpr int SAMPLES = 512;
    constexpr int SAMPLING_FREQ = 32000; // up to 16kHz, 32000 / 512 = 62.5Hz per bin
    constexpr unsigned long SAMPLING_PERIOD_US = 1000000 / SAMPLING_FREQ;

    float vReal[SAMPLES];
    float vImag[SAMPLES];

    ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, SAMPLES, SAMPLING_FREQ);
    TaskHandle_t CollectorTask;
}

namespace Collector {
    void CollectorTaskCode(void* pvParameters) {
        while (true) {
            for (int i = 0; i < SAMPLES; i++) {
                unsigned long start = micros();
                vReal[i] = Hardware::ReadMicrophone();
                vImag[i] = 0;
                while (micros() - start < SAMPLING_PERIOD_US) {
                    // wait for next sampling interval
                };
            }

            FFT.dcRemoval();
            FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.compute(FFT_FORWARD);
            FFT.complexToMagnitude();

            for (int i = 0; i < (SAMPLES / 2); i++) {
                Serial.print((int)vReal[i]);
                if (i < (SAMPLES / 2) - 1) {
                    Serial.print(",");
                }
            }
            Serial.println();
        }
    }

    void Setup() {
        xTaskCreatePinnedToCore(
            CollectorTaskCode,
            "CollectorTask",
            10000,
            NULL,
            1,
            &CollectorTask,
            1
        );
    }
}
