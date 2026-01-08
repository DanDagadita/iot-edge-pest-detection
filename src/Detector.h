#ifndef DETECTOR_H
#define DETECTOR_H

#include <Arduino.h>
#include <EloquentTinyML.h>
#include <ArduinoJson.h>
#include "pest_model.h"

namespace Detector {
    #define NUMBER_OF_INPUTS 100
    #define NUMBER_OF_OUTPUTS 1
    #define TENSOR_ARENA_SIZE 8 * 1024

    Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> pestClassifier;

    float featureBuffer[NUMBER_OF_INPUTS];
    float detectionThreshold = 0.9f;

    unsigned long lastWindowTimestamp = 0;
    int activeSignalTicks = 0;
    int totalSignalTicks = 0;
    int inferenceSkipCounter = 0;
    
    void (*onDetection)(float, float) = nullptr;

    void setup() {
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            featureBuffer[i] = 0.0f;
        }

        if (!pestClassifier.begin(pest_model_tflite)) {
            Serial.println("Error, could not initialize model");
            while (1);
        }
        Serial.println("Pest detector ready!");
    }

    void handleRemoteConfig(JsonDocument doc) {
        if (doc["threshold"].is<float>()) {
            detectionThreshold = doc["threshold"];
            Serial.printf("[Config] Updated threshold to: %.2f\n", detectionThreshold);
        }
    }

    void loop(int digitalPin, int ledPin, int windowMs) {
        if (digitalRead(digitalPin) == HIGH) {
            activeSignalTicks++;
        }

        totalSignalTicks++;

        unsigned long now = millis();
        if (now - lastWindowTimestamp >= windowMs) {
            float intensity = (float)activeSignalTicks / totalSignalTicks;

            for (int i = 0; i < NUMBER_OF_INPUTS - 1; i++) {
                featureBuffer[i] = featureBuffer[i + 1];
            }
            featureBuffer[NUMBER_OF_INPUTS - 1] = intensity;

            if (inferenceSkipCounter++ >= 10) {
                float prediction = pestClassifier.predict(featureBuffer);

                Serial.print("Intensity: ");
                Serial.print(intensity * 100);
                Serial.print("% Pest probability: ");
                Serial.println(prediction, 4);

                if (prediction > detectionThreshold) {
                    Serial.println("##### PEST DETECTED! ####");
                    digitalWrite(ledPin, HIGH);
                    if (onDetection != nullptr) {
                        onDetection(prediction, intensity);
                    }
                } else {
                    analogWrite(ledPin, (int)(intensity * 255.0));
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
