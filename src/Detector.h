#ifndef DETECTOR_H
#define DETECTOR_H

#include <EloquentTinyML.h>
#include <ArduinoJson.h>
#include "PestModel.h"
#include "Hardware.h"

namespace Detector {
    constexpr int NUMBER_OF_INPUTS = 100;
    constexpr int NUMBER_OF_OUTPUTS = 1;
    constexpr int TENSOR_ARENA_SIZE = 8 * 1024;

    Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> pestClassifier;

    float featureBuffer[NUMBER_OF_INPUTS];
    float detectionThreshold = 0.9f;

    unsigned long lastWindowTimestamp = 0;
    int activeSignalTicks = 0;
    int totalSignalTicks = 0;
    int inferenceSkipCounter = 0;

    void (*onDetection)(float, float) = nullptr;

    void handleCommandReceived(JsonDocument doc) {
        if (doc["threshold"].is<float>()) {
            detectionThreshold = doc["threshold"];
            Hardware::log("[Detector] Updated threshold to: %.2f", detectionThreshold);
        }
    }

    void setup() {
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            featureBuffer[i] = 0.0f;
        }

        if (!pestClassifier.begin(pestModelTflite)) {
            Hardware::log("[Detector] Error, could not initialize model");
            while (1);
        }
        Hardware::log("[Detector] Pest detector ready!");
    }

    void loop() {
        if (Hardware::getMicrophoneState()) {
            activeSignalTicks++;
        }

        totalSignalTicks++;

        unsigned long now = millis();
        if (now - lastWindowTimestamp >= Hardware::SAMPLING_WINDOW_MS) {
            float intensity = (float)activeSignalTicks / totalSignalTicks;

            for (int i = 0; i < NUMBER_OF_INPUTS - 1; i++) {
                featureBuffer[i] = featureBuffer[i + 1];
            }
            featureBuffer[NUMBER_OF_INPUTS - 1] = intensity;

            if (inferenceSkipCounter++ >= 10) {
                float prediction = pestClassifier.predict(featureBuffer);

                Hardware::log("[Detector] Intensity: %.2f%% Pest probability: %.4f", intensity * 100, prediction);

                if (prediction > detectionThreshold) {
                    Hardware::log("[Detector] PEST DETECTED!");
                    Hardware::setIndicator(HIGH, true);
                    if (onDetection != nullptr) {
                        onDetection(prediction, intensity);
                    }
                } else {
                    Hardware::setIndicator((int)(intensity * 255.0), false);
                }
                inferenceSkipCounter = 0;
            }

            activeSignalTicks = 0;
            totalSignalTicks = 0;
            lastWindowTimestamp = now;
        }
    }
}

#endif
