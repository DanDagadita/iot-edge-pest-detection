#include "WifiConfig.h"
#include "Hardware.h"
#include <neotimer.h>
#include <WiFiManager.h>
#include <Arduino.h>

namespace {
    constexpr int TIMER_LOOP_MS = 110;
    constexpr int TIMER_RECONNECT_MS = 2000;
    constexpr int TIMER_TIMEOUT_MS = 180000;

    constexpr const char* AP_NAME = "Pest Detector";
    constexpr const char* AP_TITLE = "Control Panel";

    bool isWifiConnected = false;

    const IPAddress AP_IP_ADDRESS(172,217,28,1);
    const IPAddress AP_IP_MASK(255,255,255,0);

    WiFiManager wifiManager;
    Neotimer timeoutTimer(TIMER_TIMEOUT_MS);

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
        static Neotimer reconnectTimer(TIMER_RECONNECT_MS);
        if (!reconnectTimer.waiting()) {
            isWifiConnected = false;
            Hardware::Log("[WiFiConfig] WiFi connection failed!");
            Hardware::PrintToLCD("WiFi connection failed!");

            WiFi.reconnect();
            reconnectTimer.start();
        }
    }
}

void (*WifiConfig::OnParamsSave)() = nullptr;

bool WifiConfig::GetIsWiFiConnected() {
    return isWifiConnected;
}

void WifiConfig::HandleAddParameter(WiFiManagerParameter* paramInstance) {
    wifiManager.addParameter(paramInstance);
}

void WifiConfig::HandleButtonPressed() {
    Hardware::Log("[WiFiConfig] Button pressed, starting WiFi config portal!");
    Hardware::PrintToLCD("Starting WiFi   config portal!");

    timeoutTimer.start();
    if (!wifiManager.getConfigPortalActive()) {
        wifiManager.startConfigPortal(AP_NAME);
    }
}

void WifiConfig::Setup() {
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

void WifiConfig::Loop() {
    static Neotimer loopTimer(TIMER_LOOP_MS);
    if (!loopTimer.waiting()) {
        wifiManager.process();
        loopTimer.start();
    }

    if (wifiManager.getConfigPortalActive() && !timeoutTimer.waiting()) {
        wifiManager.stopConfigPortal();
        if (!isWifiConnected) {
            onWiFiDisconnected(ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        }
    }
}
