#include "Detector.h"
#include "PestModel.h"
#include "Hardware.h"
#include "MqttHandler.h"
#include "EloquentTinyML.h"
#include <ArduinoJson.h>
#include <arduinoFFT.h>
#include <Arduino.h>
#include <cmath>

namespace {
    constexpr int SAMPLES = 512;
    constexpr int SAMPLING_FREQ = 32000;
    constexpr unsigned long SAMPLING_PERIOD_US = 1000000 / SAMPLING_FREQ;

    constexpr int ARENA_SIZE = 10000;
    constexpr int NUMBER_OF_INPUTS = SAMPLES / 2;
    constexpr int OUTPUT_SIZE = 1;
    constexpr int LOOP_LIMIT_MS = 80;

    float vReal[SAMPLES];
    float vImag[SAMPLES];
    float featureBuffer[NUMBER_OF_INPUTS];
    float preprocessedBuffer[NUMBER_OF_INPUTS];

    ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, SAMPLES, SAMPLING_FREQ);
    Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, OUTPUT_SIZE, ARENA_SIZE> tf;

    float detectionThreshold = 0.5f;
    TaskHandle_t DetectorTask;

    float calculateIntensity() {
        float sum = 0;
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            sum += featureBuffer[i];
        }
        return sum / NUMBER_OF_INPUTS;
    }

    void preprocessFeatures(const float* input, float* output) {
        const float eps = 1e-8f;
        
        float mean = 0;
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            mean += input[i];
        }
        mean /= NUMBER_OF_INPUTS;
        
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            output[i] = input[i] - mean;
        }

        float sum_sq = 0;
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            sum_sq += output[i] * output[i];
        }
        float rms = sqrt(sum_sq / NUMBER_OF_INPUTS) + eps;

        if (rms > eps) {
            for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
                output[i] = output[i] / (rms + 0.1f);
            }
        }

        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            float val = output[i];
            output[i] = (val >= 0 ? 1.0f : -1.0f) * log1p(fabs(val) * 5.0f);
        }

        float max_val = eps;
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            float abs_val = fabs(output[i]);
            if (abs_val > max_val) max_val = abs_val;
        }

        if (max_val > eps) {
            for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
                output[i] = output[i] / (max_val + 0.1f);
            }
        }
    }

    void handlePrintToLCD(const float prediction, const float intensity) {
        static unsigned long lastLoopLCD = 0;
        if (millis() - lastLoopLCD > LOOP_LIMIT_MS) {
            if (prediction > detectionThreshold) {
                Hardware::PrintToLCD("PEST DETECTED!");
            } else {
                int intPercent = constrain((int)(intensity / 5.0f), 0, 100);
                Hardware::PrintToLCD("Int: %3d%% Prob: %.2f", intPercent, prediction);
            }
            lastLoopLCD = millis();
        }
    }

    void handleSetIndicator(const float prediction, const float intensity) {
        if (prediction > detectionThreshold) {
            Hardware::Log("[Detector] PEST DETECTED! Prob: %.1f%%", prediction);
            Hardware::SetIndicator(HIGH, true);
            if (Detector::OnDetection) {
                Detector::OnDetection(prediction, intensity);
            }
        } else {
            int brightness = constrain((int)(prediction * 255.0f), 0, 255);
            Hardware::SetIndicator(brightness, false);
        }
    }
}

namespace Detector {
    void (*OnDetection)(const float, const float) = nullptr;

    void HandleConfigReceived(const JsonDocument& doc) {
        if (doc["threshold"].is<float>()) {
            detectionThreshold = doc["threshold"];
            Hardware::Log("[Detector] Threshold: %.4f", detectionThreshold);
        }
    }

    void DetectorTaskCode(void* pvParameters) {
        while (true) {
            if (!MqttHandler::GetIsMqttConnected()) {
                vTaskDelay(pdMS_TO_TICKS(100));
                continue;
            }

            for (int i = 0; i < SAMPLES; i++) {
                unsigned long start = micros();
                vReal[i] = (float)Hardware::ReadMicrophone();
                vImag[i] = 0;
                while (micros() - start < SAMPLING_PERIOD_US) {
                    // wait for next sampling interval
                };
            }

            FFT.dcRemoval();
            FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.compute(FFT_FORWARD);
            FFT.complexToMagnitude();
            
            for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
                featureBuffer[i] = vReal[i];
            }

            preprocessFeatures(featureBuffer, preprocessedBuffer);

            float prediction = tf.predict(preprocessedBuffer);
            float intensity = calculateIntensity();

            handlePrintToLCD(prediction, intensity);
            handleSetIndicator(prediction, intensity);
        }
    }

    void Setup() {
        tf.begin(pestModelTflite);

        xTaskCreatePinnedToCore(
            DetectorTaskCode,
            "DetectorTask",
            10000,
            NULL,
            1,
            &DetectorTask,
            1
        );
        Hardware::Log("[Detector] Started FFT Inference Task");
    }
}
