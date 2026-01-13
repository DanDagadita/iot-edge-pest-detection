#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

class WiFiManagerParameter;

namespace WifiConfig {
    extern void (*OnParamsSave)();

    bool GetIsWiFiConnected();

    void HandleAddParameter(WiFiManagerParameter* paramInstance);
    void HandleButtonPressed();

    void Setup();
    void Loop();
}

#endif
