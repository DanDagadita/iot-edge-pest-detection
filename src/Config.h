#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

namespace Config {
    #define KEY_CONFIG "config"
    #define AP_NAME "Pest Detector"
    #define AP_TITLE "Control Panel"
    #define AP_IP_MASK 255,255,255,0
    #define AP_IP_ADDRESS 172,217,28,1
    #define TIMEOUT 15

    struct ConfigParam {
        const char* key;
        const char* label;
        int length;
        char value[64];
        WiFiManagerParameter* paramInstance;
    };

    ConfigParam portalParams[] = {
        {"mqtt_server", "MQTT Server", 40, "", nullptr},
        {"mqtt_port", "MQTT Port", 6, "", nullptr},
        {"mqtt_username", "MQTT Username", 40, "", nullptr},
        {"mqtt_password", "MQTT Password", 40, "", nullptr},
        {"user_token", "User Token", 64, "", nullptr}
    };

    const int portalParamCount = sizeof(portalParams) / sizeof(portalParams[0]);

    Preferences persistentStore;
    WiFiManager portalManager;
    bool isWifiConnected = false;

    void (*onConfigChange)() = nullptr;

    void saveParamsCallback() {
        persistentStore.begin(KEY_CONFIG, false);
        for (int i = 0; i < portalParamCount; i++) {
            strcpy(portalParams[i].value, portalParams[i].paramInstance->getValue());
            persistentStore.putString(portalParams[i].key, portalParams[i].value);
        }
        persistentStore.end();

        Serial.println("[Config] Settings Saved.");
        if (onConfigChange != nullptr) {
            onConfigChange();
        }
    }

    // Helper to get value by key later in Detector.h
    const char* getParamValue(const char* key) {
        for (int i = 0; i < portalParamCount; i++) {
            if (strcmp(portalParams[i].key, key) == 0) {
                return portalParams[i].value;
            }
        }
        return "";
    }

    void setup() {
        WiFi.mode(WIFI_STA);

        // Load preferences
        persistentStore.begin(KEY_CONFIG, true);
        for (int i = 0; i < portalParamCount; i++) {
            String storedVal = persistentStore.getString(portalParams[i].key, "");
            strcpy(portalParams[i].value, storedVal.c_str());

            portalParams[i].paramInstance = new WiFiManagerParameter(
                portalParams[i].key, portalParams[i].label, portalParams[i].value, portalParams[i].length
            );
            portalManager.addParameter(portalParams[i].paramInstance);
        }
        persistentStore.end();

        // Menu config
        const char* menuLayout[] = {"wifi", "info", "param", "sep", "restart"};
        portalManager.setMenu(menuLayout, 5);
        portalManager.setShowInfoUpdate(false);
        portalManager.setTitle(AP_TITLE);

        // WiFiManager config
        portalManager.setConfigPortalBlocking(true); // block everything until after the connection is completed
        portalManager.setSaveParamsCallback(saveParamsCallback);
        portalManager.setConnectTimeout(TIMEOUT);
        portalManager.setAPStaticIPConfig(IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_MASK));
        portalManager.setConfigPortalTimeout(180); // in case the connection to the router is interrupted, unless this is set to non-zero it will not attempt to reconnect

        if (!portalManager.autoConnect(AP_NAME)) {
            portalManager.reboot();
        } else {
            isWifiConnected = true;
            portalManager.setConfigPortalTimeout(0); // this is no longer needed since the connection is done
            portalManager.setConfigPortalBlocking(false); // disable blocking and then start the web server for the config portal
            portalManager.startConfigPortal(AP_NAME);
        }
    }

    void loop() {
        if (!isWifiConnected) {
            return;
        }

        portalManager.process();

        if (!WiFi.isConnected()) {
            portalManager.reboot();
        }
    }
}

#endif
