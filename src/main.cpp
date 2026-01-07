#include <Arduino.h>

const int DIGITAL_PIN = 32;
const int LED_PIN = 2;
const int WINDOW_MS = 10;

unsigned long last_window_time = 0;

int high_count = 0;
int total_count = 0;

void setup() {
  Serial.begin(115200);

  pinMode(DIGITAL_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("timestamp_ms,intensity");
}

void loop() {
  int val = digitalRead(DIGITAL_PIN);

  if (val == HIGH) {
    high_count++;
  }

  total_count++;

  unsigned long now = millis();
  if (now - last_window_time >= WINDOW_MS) {

    float intensity = (float)high_count / total_count * 100.0;

    Serial.print(now);
    Serial.print(",");
    Serial.println(intensity, 2);

    analogWrite(LED_PIN, (int)(intensity * 2.55)); 

    high_count = 0;
    total_count = 0;
    last_window_time = now;
  }
}
