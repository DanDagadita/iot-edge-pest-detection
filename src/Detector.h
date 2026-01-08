#ifndef DETECTOR_H
#define DETECTOR_H

#include <Arduino.h>

#include <EloquentTinyML.h>
#include <ArduinoJson.h>

#include "pest_model.h"

#define NUMBER_OF_INPUTS 100
#define NUMBER_OF_OUTPUTS 1
#define TENSOR_ARENA_SIZE 8 * 1024

namespace Detector {
    Eloquent::TinyML::TfLite<NUMBER_OF_INPUTS, NUMBER_OF_OUTPUTS, TENSOR_ARENA_SIZE> ml;

    float threshold = 0.9;
    void (*onDetection)(float, float) = nullptr;
    float input_buffer[NUMBER_OF_INPUTS];
    unsigned long last_window_time = 0;
    int high_count = 0;
    int total_count = 0;
    int inference_throttle = 0;

    void setup() {
        for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
            input_buffer[i] = 0.0f;
        }

        if (!ml.begin(pest_model_tflite)) {
            Serial.println("Error, could not initialize model");
            while (1);
        }
        Serial.println("Pest detector ready!");
    }

    void loop(int digitalPin, int ledPin, int windowMs) {
        if (digitalRead(digitalPin) == HIGH) {
            high_count++;
        }

        total_count++;

        unsigned long now = millis();
        if (now - last_window_time >= windowMs) {
            float intensity = (float)high_count / total_count;

            for (int i = 0; i < NUMBER_OF_INPUTS - 1; i++) {
                input_buffer[i] = input_buffer[i + 1];
            }
            input_buffer[NUMBER_OF_INPUTS - 1] = intensity;

            if (inference_throttle++ >= 10) {
                float prediction = ml.predict(input_buffer);

                Serial.print("Intensity: ");
                Serial.print(intensity * 100);
                Serial.print("% Pest probability: ");
                Serial.println(prediction, 4);

                if (prediction > threshold) {
                    Serial.println("##### PEST DETECTED! ####");
                    digitalWrite(ledPin, HIGH);
                    if (onDetection != nullptr) {
                        onDetection(prediction, intensity);
                    }
                } else {
                    analogWrite(ledPin, (int)(intensity * 255.0));
                }
                inference_throttle = 0;
            }

            high_count = 0;
            total_count = 0;
            last_window_time = now;
        }
    }

    void handleCallback(JsonDocument doc) {
        if (doc["threshold"].is<float>()) {
            threshold = doc["threshold"];
            Serial.printf("[Config] Updated threshold to: %.2f\n", threshold);
        }
    }
}

#endif
