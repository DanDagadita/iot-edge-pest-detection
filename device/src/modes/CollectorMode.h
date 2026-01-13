#include "Hardware.h"
#include "Collector.h"
#include "Arduino.h"

void setup() {
    Hardware::Setup();
    Collector::Setup();
}

void loop() {
    Hardware::Loop();
    Collector::Loop();
}
