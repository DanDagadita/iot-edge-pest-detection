#include "Arduino.h"
#include "Pitches.h"

constexpr int SPEAKER_PIN = 25;

void playTone(int frequency, int duration) {
  ledcWriteTone(0, frequency);
  delay(duration);
  ledcWriteTone(0, 0);
}

int melody[] = {
  NOTE_G4, REST, NOTE_G4, REST, NOTE_F4, REST, NOTE_E4, REST,
  NOTE_G4, REST, NOTE_C5, REST, NOTE_B4, REST, NOTE_A4, REST, NOTE_G4, REST, NOTE_G4, REST, NOTE_E4, REST,
  NOTE_G4, REST, NOTE_G4, REST, NOTE_F4, REST, NOTE_E4, REST,
  NOTE_G4, REST, NOTE_G4, REST, NOTE_G4, REST, NOTE_A4, REST, NOTE_G4, REST, NOTE_E4, REST, NOTE_G4, REST, NOTE_E4, REST, NOTE_G4, REST, NOTE_C5, REST, NOTE_E5, REST, NOTE_E5, REST, NOTE_E5, REST, NOTE_D5, REST, NOTE_C5, REST, NOTE_B5, REST, NOTE_C5
};

int durations[] = {
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8,
  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

void setup()
{
  ledcSetup(0, 1000, 8); // channel 0, 1kHz, 8-bit resolution
  ledcAttachPin(SPEAKER_PIN, 0); // attach pin to channel 0
}

void loop()
{
  int size = sizeof(durations) / sizeof(int);

  for (int note = 0; note < size; note++) {
    int duration = 1000 / durations[note];
    playTone(melody[note], duration);

    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);

    ledcWrite(0, 0); // silence
  }
}
