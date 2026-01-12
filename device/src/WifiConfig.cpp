#include "WifiConfig.h"
#include "Hardware.h"
#include <WiFiManager.h>
#include <Arduino.h>

namespace {
    constexpr int LOOP_LIMIT_MS = 110;
    constexpr int LOOP_LIMIT_RECONNECT_MS = 2000;
    constexpr int PORTAL_TIMEOUT_MS = 180000;
    constexpr const char* AP_NAME = "Pest Detector";
    constexpr const char* AP_TITLE = "Control Panel";

    bool isWifiConnected = false;
    unsigned long portalStartTime = 0;

    const IPAddress AP_IP_ADDRESS(172,217,28,1);
    const IPAddress AP_IP_MASK(255,255,255,0);

    WiFiManager wifiManager;

    void onWiFiConnected(WiFiEvent_t event) {
        if (event != ARDUINO_EVENT_WIFI_STA_CONNECTED) {
            return;
        }
        isWifiConnected = true;
        Hardware::Log("[WiFiConfig] WiFi connected!");
        Hardware::PrintToLCD("WiFi connected!");
    }

    void onWiFiDisconnected(WiFiEvent_t event) {
        if (event != ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            return;
        }
        static unsigned long lastLoopReconnect = 0;
        if (lastLoopReconnect == 0 || millis() - lastLoopReconnect > LOOP_LIMIT_RECONNECT_MS) {
            isWifiConnected = false;
            Hardware::Log("[WiFiConfig] WiFi connection failed!");
            Hardware::PrintToLCD("WiFi connection failed!");

            WiFi.reconnect();
            lastLoopReconnect = millis();
        }
    }
}

namespace WifiConfig {
    void (*OnParamsSave)() = nullptr;

    bool GetIsWiFiConnected() {
        return isWifiConnected;
    }

    void HandleAddParameter(WiFiManagerParameter* paramInstance) {
        wifiManager.addParameter(paramInstance);
    }

    void HandleButtonPressed() {
        Hardware::Log("[WiFiConfig] Button pressed, starting WiFi config portal!");
        Hardware::PrintToLCD("Starting WiFi   config portal!");

        portalStartTime = millis();
        if (!wifiManager.getConfigPortalActive()) {
            wifiManager.startConfigPortal(AP_NAME);
        }
    }

    void Setup() {
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
        wifiManager.setSaveParamsCallback(OnParamsSave);
        wifiManager.setAPStaticIPConfig(AP_IP_ADDRESS, AP_IP_ADDRESS, AP_IP_MASK);

        if (wifiManager.getWiFiIsSaved()) {
            WiFi.begin(wifiManager.getWiFiSSID(), wifiManager.getWiFiPass());
        } else {
            Hardware::Log("[WiFiConfig] No WiFi configuration saved, press button to open config portal");
            Hardware::PrintToLCD("No WiFi config  saved, press btn");
        }
    }

    void Loop() {
        static unsigned long lastLoop = 0;
        if (lastLoop == 0 || millis() - lastLoop > LOOP_LIMIT_MS) {
            wifiManager.process();
            lastLoop = millis();
        }

        if (wifiManager.getConfigPortalActive() && millis() - portalStartTime > PORTAL_TIMEOUT_MS) {
            wifiManager.stopConfigPortal();
            if (!isWifiConnected) {
                onWiFiDisconnected(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
            }
        }
    }
}
