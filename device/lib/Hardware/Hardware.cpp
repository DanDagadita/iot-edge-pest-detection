#include "Hardware.h"
#include <LiquidCrystal_I2C.h>
#include <EasyButton.h>
#include <Arduino.h>

namespace {
    constexpr int PIN_SENSOR = 32;
    constexpr int PIN_LED = 2;
    constexpr int PIN_BUTTON = 0;

    constexpr int LCD_COLUMNS = 16;
    constexpr int LCD_ROWS = 2;

    constexpr int BUTTON_RESET_MS = 3000;

    EasyButton button(PIN_BUTTON);
    LiquidCrystal_I2C lcd(0x27, LCD_COLUMNS, LCD_ROWS);
}

void (*Hardware::OnButtonPressed)() = nullptr;

void Hardware::PrintToLCD(const char* format, ...) {
    char buffer[33];
    va_list args;
    va_start(args, format);
    
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    int actualLen = strlen(buffer);

    char line1[17]; 
    for (int i = 0; i < 16; i++) {
        line1[i] = (i < actualLen) ? buffer[i] : ' ';
    }
    line1[16] = '\0';

    char line2[17];
    for (int i = 0; i < 16; i++) {
        int bufIndex = i + 16;
        line2[i] = (bufIndex < actualLen) ? buffer[bufIndex] : ' ';
    }
    line2[16] = '\0';

    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void Hardware::Log(const char* format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    Serial.println(buffer);
}

bool Hardware::GetMicrophoneState() {
    return digitalRead(PIN_SENSOR) == HIGH;
}

void Hardware::SetIndicator(const int value, const bool isDigital) {
    if (isDigital) {
        digitalWrite(PIN_LED, HIGH);
    } else {
        analogWrite(PIN_LED, value);
    }
}

void Hardware::Setup() {
    Serial.begin(921600);
    pinMode(PIN_SENSOR, INPUT);
    pinMode(PIN_LED, OUTPUT);
    lcd.init();
    delay(100);
    PrintToLCD("Loading...");
    lcd.backlight();
    button.begin();
    button.onPressed(OnButtonPressed);
}

void Hardware::Loop() {
    button.read();
}
