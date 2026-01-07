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
        {"mqtt_server",   "MQTT Server",   40, "", NULL},
        {"mqtt_port",     "MQTT Port",     6,  "", NULL},
        {"mqtt_username", "MQTT Username", 40, "", NULL},
        {"mqtt_password", "MQTT Password", 40, "", NULL},
        {"mqtt_topic",    "MQTT Topic",    40, "", NULL},
        {"user_token",    "User Token",    64, "", NULL}
    };
    
    const int paramCount = sizeof(params) / sizeof(params[0]);
    Preferences preferences;
    WiFiManager wm;
    bool connection_success = false;

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

        // WiFiManager config
        wm.setConfigPortalBlocking(true);
        wm.setSaveParamsCallback(saveParamsCallback);
        wm.setConnectTimeout(TIMEOUT);
        wm.setAPStaticIPConfig(IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_ADDRESS), IPAddress(AP_IP_MASK));
        wm.setTitle(AP_TITLE);

        if (!wm.autoConnect(AP_NAME)) {
            wm.reboot();
        } else {
            connection_success = true;
            wm.setConfigPortalBlocking(false);
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
