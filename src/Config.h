#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>

namespace Config {
    #define TIMEOUT 15
    #define AP_IP_ADDRESS 172,217,28,1
    #define AP_IP_MASK 255,255,255,0
    #define KEY_CONFIG "config"
    #define AP_NAME "Pest Detector"
    #define AP_TITLE "Control Panel"

    struct ConfigParam {
        const char* key;
        const char* label;
        int length;
        char value[64];
        WiFiManagerParameter* param;
    };

    ConfigParam params[] = {
        {"mqtt_server", "MQTT Server", 40, "", nullptr},
        {"mqtt_port", "MQTT Port", 6, "", nullptr},
        {"mqtt_username", "MQTT Username", 40, "", nullptr},
        {"mqtt_password", "MQTT Password", 40, "", nullptr},
        {"user_token", "User Token", 64, "", nullptr}
    };

    const int paramCount = sizeof(params) / sizeof(params[0]);
    Preferences preferences;
    WiFiManager wm;
    bool connection_success = false;
    void (*onConfigChange)() = nullptr;

    // Helper to get value by key later in Detector.h
    const char* get(const char* key) {
        for (int i = 0; i < paramCount; i++) {
            if (strcmp(params[i].key, key) == 0) {
                return params[i].value;
            }
        }
        return "";
    }

    void saveParamsCallback() {
        preferences.begin(KEY_CONFIG, false);
        for (int i = 0; i < paramCount; i++) {
            strcpy(params[i].value, params[i].param->getValue());
            preferences.putString(params[i].key, params[i].value);
        }
        preferences.end();
        if (onConfigChange != nullptr) {
            onConfigChange();
        }
        Serial.println("[Config] Settings Saved.");
    }

    void setup() {
        WiFi.mode(WIFI_STA);
        
        // Load preferences
        preferences.begin(KEY_CONFIG, true);
        for (int i = 0; i < paramCount; i++) {
            String storedVal = preferences.getString(params[i].key, "");
            strcpy(params[i].value, storedVal.c_str());

            params[i].param = new WiFiManagerParameter(params[i].key, params[i].label, params[i].value, params[i].length);
            wm.addParameter(params[i].param);
        }
        preferences.end();

        // Menu config
        const char* menu[] = {"wifi", "info", "param", "sep", "erase", "restart"};
        wm.setMenu(menu, 6);
        wm.setShowInfoUpdate(false);
        wm.setShowInfoErase(false);
        wm.setTitle(AP_TITLE);

        // WiFiManager config
        wm.setConfigPortalBlocking(true); // block everything until after the connection is completed
        wm.setSaveParamsCallback(saveParamsCallback);
        wm.setConnectTimeout(TIMEOUT);
        wm.setAPStaticIPConfig(IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_MASK));
        wm.setConfigPortalTimeout(180); // in case the connection to the router is interrupted, unless this is set to non-zero it will not attempt to reconnect

        if (!wm.autoConnect(AP_NAME)) {
            wm.reboot();
        } else {
            connection_success = true;
            wm.setConfigPortalTimeout(0); // this is no longer needed since the connection is done
            wm.setConfigPortalBlocking(false); // disable blocking and then start the web server for the config portal
            wm.startConfigPortal(AP_NAME);
        }
    }

    void loop() {
        if (!connection_success) {
            return;
        }

        wm.process();

        if (!WiFi.isConnected()) {
            wm.reboot();
        }
    }
}

#endif
