#ifndef DETECTOR_H
#define DETECTOR_H

#include <EloquentTinyML.h>
#include <ArduinoJson.h>
#include "PestModel.h"
#include "Hardware.h"

namespace Detector {
    constexpr int LOOP_LIMIT_MS = 80;
    constexpr int NUMBER_OF_INPUTS = 100;
    constexpr int NUMBER_OF_OUTPUTS = 1;
    constexpr int TENSOR_ARENA_SIZE = 8 * 1024;

    Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> pestClassifier;

    float featureBuffer[NUMBER_OF_INPUTS];
    float detectionThreshold = 0.9f;
    unsigned int window = 10;

    int activeSignalTicks = 0;
    int totalSignalTicks = 0;
    int inferenceSkipCounter = 0;

    void (*onDetection)(const float, const float) = nullptr;

    void handleConfigReceived(JsonDocument doc) {
        if (doc["threshold"].is<float>() && doc["window"].is<unsigned int>()) {
            detectionThreshold = doc["threshold"];
            window = doc["window"];
            Hardware::log("[Detector] Updated threshold %.2f, window %d", detectionThreshold, window);
        }
    }

    void setup() {
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            featureBuffer[i] = 0.0f;
        }

        if (!pestClassifier.begin(pestModelTflite)) {
            Hardware::log("[Detector] Error, could not initialize model");
            Hardware::printToLCD("Error, could not init model");
            while (1);
        }
    }

    void handlePrintToLCD(float prediction, float intensity) {
        static unsigned long lastLoopLCD = 0;
        if (lastLoopLCD == 0 || millis() - lastLoopLCD > LOOP_LIMIT_MS) {
            if (prediction > detectionThreshold) {
                Hardware::printToLCD("PEST DETECTED!"); // 60ms
            } else {
                Hardware::printToLCD("Intensity: %3d%% Prob: %.4f", (int)(intensity * 100), prediction); // 60ms
            }
            lastLoopLCD = millis();
        }
    }

    void handleSetIndicator(float prediction, float intensity) {
        if (prediction > detectionThreshold) {
            Hardware::log("[Detector] PEST DETECTED!");
            Hardware::setIndicator(HIGH, true);
            if (onDetection != nullptr) {
                onDetection(prediction, intensity); // 5ms, MQTT publish to telemetry topic
            }
        } else {
            Hardware::setIndicator((int)(intensity * 255.0), false);
        }
    }

    void loop() {
        static int activeSignalTicks = 0;
        static int totalSignalTicks = 0;

        if (Hardware::getMicrophoneState()) {
            activeSignalTicks++;
        }

        totalSignalTicks++;

        unsigned long now = millis();
        static unsigned long lastLoop = 0;
        if (lastLoop == 0 || now - lastLoop >= Hardware::SAMPLING_WINDOW_MS) {
            float intensity = (float)activeSignalTicks / totalSignalTicks;

            for (int i = 0; i < NUMBER_OF_INPUTS - 1; i++) {
                featureBuffer[i] = featureBuffer[i + 1];
            }
            featureBuffer[NUMBER_OF_INPUTS - 1] = intensity;

            if (inferenceSkipCounter++ >= 10) {
                float prediction = pestClassifier.predict(featureBuffer);

                Hardware::log("[Detector] Intensity: %.2f%% Pest probability: %.4f", intensity * 100, prediction); // 0.8ms

                handlePrintToLCD(prediction, intensity);
                handleSetIndicator(prediction, intensity);
                inferenceSkipCounter = 0;
            }

            activeSignalTicks = 0;
            totalSignalTicks = 0;
            lastLoop = millis();
        }
    }
}

#endif
