#ifndef HARDWARE_H
#define HARDWARE_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <EasyButton.h>

namespace Hardware {
    constexpr int PIN_SENSOR = 32;
    constexpr int PIN_LED = 2;
    constexpr int PIN_BUTTON = 0;
    constexpr int BUTTON_RESET_MS = 3000;
    constexpr int SAMPLING_WINDOW_MS = 10;
    constexpr int LCD_COLUMNS = 16;
    constexpr int LCD_ROWS = 2;

    EasyButton button(PIN_BUTTON);
    LiquidCrystal_I2C lcd(0x27, LCD_COLUMNS, LCD_ROWS);

    void (*onButtonPressed)() = nullptr;

    void printToLCD(const char* str) {
        if (str == nullptr) return;

        int totalLen = strlen(str);

        char line1[17]; 
        for (int i = 0; i < 16; i++) {
            line1[i] = (i < totalLen) ? str[i] : ' ';
        }
        line1[16] = '\0';

        char line2[17];
        for (int i = 0; i < 16; i++) {
            int strIndex = i + 16;
            line2[i] = (strIndex < totalLen) ? str[strIndex] : ' ';
        }
        line2[16] = '\0';

        lcd.setCursor(0, 0);
        lcd.print(line1);
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }

    template<typename... Args>
    void printToLCD(const char* format, Args... args) {
        char fullBuffer[33]; 

        snprintf(fullBuffer, sizeof(fullBuffer), format, args...);

        int totalLen = strlen(fullBuffer);

        char line1[17];
        int copyLen1 = (totalLen > 16) ? 16 : totalLen;
        memcpy(line1, fullBuffer, copyLen1);
        
        for (int i = copyLen1; i < 16; i++) {
            line1[i] = ' ';
        }
        line1[16] = '\0';

        char line2[17];
        int copyLen2 = 0;
        if (totalLen > 16) {
            copyLen2 = totalLen - 16;
            if (copyLen2 > 16) copyLen2 = 16; 
            memcpy(line2, fullBuffer + 16, copyLen2);
        }

        for (int i = copyLen2; i < 16; i++) {
            line2[i] = ' ';
        }
        line2[16] = '\0';

        lcd.setCursor(0, 0);
        lcd.print(line1);
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }

    inline void log(const char* str) {
        Serial.write(str);
        Serial.write('\n');
    }

    template<typename... Args>
    inline void log(const char* format, Args&&... args) {
        Serial.printf(format, std::forward<Args>(args)...);
        Serial.write('\n');
    }

    bool getMicrophoneState() {
        return digitalRead(PIN_SENSOR) == HIGH;
    }

    void setIndicator(const int value, const bool isDigital) {
        if (isDigital) {
            digitalWrite(PIN_LED, HIGH);
        } else {
            analogWrite(PIN_LED, value);
        }
    }

    void setup() {
        Serial.begin(115200);
        pinMode(PIN_SENSOR, INPUT);
        pinMode(PIN_LED, OUTPUT);
        lcd.init();
        delay(100);
        printToLCD("Loading...");
        lcd.backlight();
        button.begin();
        button.onPressed(onButtonPressed);
    }

    void loop() {
        button.read();
    }
}

#endif
