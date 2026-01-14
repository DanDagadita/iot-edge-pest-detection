
#include "Detector.h"
#include "Hardware.h"
#include "PestModel.h"
#include "MqttHandler.h"
#include <neotimer.h>
#include <EloquentTinyML.h>
#include <ArduinoJson.h>

namespace {
    constexpr int TIMER_LOOP_MS = 80;
    constexpr int NUMBER_OF_INPUTS = 100;
    constexpr int NUMBER_OF_OUTPUTS = 1;
    constexpr int TENSOR_ARENA_SIZE = 8 * 1024;

    Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> tf;
    TaskHandle_t DetectorTask;

    float featureBuffer[NUMBER_OF_INPUTS];
    float detectionThreshold = 0.9f;
    unsigned int window = 10;

    void handlePrintToLCD(const float prediction, const float intensity) {
        static Neotimer loopTimer(TIMER_LOOP_MS);
        if (!loopTimer.waiting()) {
            if (prediction > detectionThreshold) {
                Hardware::PrintToLCD("PEST DETECTED!"); // 60ms
            } else {
                Hardware::PrintToLCD("Intensity: %3d%% Prob: %.4f", (int)(intensity * 100), prediction); // 60ms
            }
            loopTimer.start();
        }
    }

    void handleSetIndicator(const float prediction, const float intensity) {
        if (prediction > detectionThreshold) {
            Hardware::Log("[Detector] PEST DETECTED!");
            Hardware::SetIndicator(HIGH, true);
            if (Detector::OnDetection != nullptr) {
                Detector::OnDetection(prediction, intensity); // 5ms, MQTT publish to telemetry topic
            }
        } else {
            Hardware::SetIndicator((int)(intensity * 255.0), false);
        }
    }

    void detectorTaskCode(void* pvParameters) {
        while (true) {
            if (!MqttHandler::GetIsMqttConnected()) {
                continue;
            }

            static int activeSignalTicks = 0;
            static int totalSignalTicks = 0;

            if (Hardware::GetMicrophoneState()) {
                activeSignalTicks++;
            }

            totalSignalTicks++;

            static Neotimer samplingTimer(Hardware::SAMPLING_WINDOW_MS);
            if (!samplingTimer.waiting()) {
                float intensity = (float)activeSignalTicks / totalSignalTicks;

                for (int i = 0; i < NUMBER_OF_INPUTS - 1; i++) {
                    featureBuffer[i] = featureBuffer[i + 1];
                }
                featureBuffer[NUMBER_OF_INPUTS - 1] = intensity;

                static int inferenceSkipCounter = 0;
                if (inferenceSkipCounter++ >= 10) {
                    float prediction = tf.predict(featureBuffer);

                    Hardware::Log("[Detector] Intensity: %.2f%% Pest probability: %.4f", intensity * 100, prediction); // 0.8ms

                    handlePrintToLCD(prediction, intensity);
                    handleSetIndicator(prediction, intensity);

                    inferenceSkipCounter = 0;
                }

                activeSignalTicks = 0;
                totalSignalTicks = 0;
                samplingTimer.start();
            }
        }
    }
}

void (*Detector::OnDetection)(const float, const float) = nullptr;

void Detector::HandleConfigReceived(const JsonDocument& doc) {
    if (doc["threshold"].is<float>() && doc["window"].is<unsigned int>()) {
        detectionThreshold = doc["threshold"];
        window = doc["window"];
        Hardware::Log("[Detector] Updated threshold %.2f, window %d", detectionThreshold, window);
    }
}

void Detector::Setup() {
    for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
        featureBuffer[i] = 0.0f;
    }

    if (!tf.begin(pestModelTflite)) {
        Hardware::Log("[Detector] Error, could not initialize model");
        Hardware::PrintToLCD("Error, could not init model");
        while (1);
    }

    xTaskCreatePinnedToCore(
        detectorTaskCode,
        "DetectorTask",
        10000,
        NULL,
        1,
        &DetectorTask,
        1
    );
}
