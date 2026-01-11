#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFiManager.h>
#include "Hardware.h"

namespace WiFiConfig {
    unsigned long portalStartTime = 0;
    constexpr int PORTAL_TIMEOUT_MS = 180000;

    constexpr const char* AP_NAME = "Pest Detector";
    constexpr const char* AP_TITLE = "Control Panel";

    bool isWifiConnected = false;

    const IPAddress AP_IP_ADDRESS(172,217,28,1);
    const IPAddress AP_IP_MASK(255,255,255,0);

    WiFiManager wifiManager;

    void (*onParamsSave)() = nullptr;

    void handleAddParameter(WiFiManagerParameter* paramInstance) {
        wifiManager.addParameter(paramInstance);
    }

    void handleButtonPressed() {
        Hardware::log("[WiFiConfig] Button pressed, starting WiFi config portal!");
        Hardware::printToLCD("Starting WiFi   config portal!");

        portalStartTime = millis();
        if (!wifiManager.getConfigPortalActive()) {
            wifiManager.startConfigPortal(AP_NAME);
        }
    }

    void onWiFiConnected(WiFiEvent_t event) {
        if (event != ARDUINO_EVENT_WIFI_STA_CONNECTED) {
            return;
        }
        isWifiConnected = true;
        Hardware::log("[WiFiConfig] WiFi connected!");
        Hardware::printToLCD("WiFi connected!");
    }

    void onWiFiDisconnected(WiFiEvent_t event) {
        if (event != ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            return;
        }
        isWifiConnected = false;
        Hardware::log("[WiFiConfig] WiFi connection failed!");
        Hardware::printToLCD("WiFi connection failed!");

        WiFi.reconnect();
    }

    void setup() {
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(false);
        WiFi.onEvent(onWiFiConnected);
        WiFi.onEvent(onWiFiDisconnected);

        // Menu config
        const char* menuLayout[] = {"wifi", "info", "param", "sep", "restart"};
        wifiManager.setMenu(menuLayout, 5);
        wifiManager.setShowInfoUpdate(false);
        wifiManager.setTitle(AP_TITLE);

        // WiFiManager config
        wifiManager.setConfigPortalBlocking(false);
        wifiManager.setSaveParamsCallback(onParamsSave);
        wifiManager.setAPStaticIPConfig(AP_IP_ADDRESS, AP_IP_ADDRESS, AP_IP_MASK);

        WiFi.begin(wifiManager.getWiFiSSID(), wifiManager.getWiFiPass());
    }

    void loop() {
        wifiManager.process();

        if (wifiManager.getConfigPortalActive() && millis() - portalStartTime > PORTAL_TIMEOUT_MS) {
            wifiManager.stopConfigPortal();
            if (!isWifiConnected) {
                onWiFiDisconnected(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
            }
        }
    }
}

#endif
