#ifndef CONFIG_H
#define CONFIG_H

#include <WiFiManager.h>
#include <Preferences.h>
#include "Hardware.h"

namespace Config {
    constexpr const char* KEY_CONFIG = "config";

    struct Settings {
        char mqttServer[40];
        char mqttPort[6];
        char mqttUsername[40];
        char mqttPassword[40];
        char userToken[64];
    } settings;

    struct Param {
        const char* key;
        const char* label;
        int length;
        char* value;
        WiFiManagerParameter* paramInstance;
    };

    Param params[] = {
        {"mqtt_server", "MQTT Server", 40, settings.mqttServer},
        {"mqtt_port", "MQTT Port", 6, settings.mqttPort},
        {"mqtt_username", "MQTT Username", 40, settings.mqttUsername},
        {"mqtt_password", "MQTT Password", 40, settings.mqttPassword},
        {"user_token", "User Token", 64, settings.userToken}
    };

    const int portalParamCount = sizeof(params) / sizeof(params[0]);

    Preferences persistentStore;

    void (*onConfigChange)() = nullptr;
    void (*onAddParameter)(WiFiManagerParameter*) = nullptr;

    void handleParamsSave() {
        persistentStore.begin(KEY_CONFIG, false);
        for (int i = 0; i < portalParamCount; i++) {
            strcpy(params[i].value, params[i].paramInstance->getValue());
            persistentStore.putString(params[i].key, params[i].value);
        }
        persistentStore.end();

        Hardware::log("[Config] Settings saved");
        onConfigChange();
    }

    void setup() {
        persistentStore.begin(KEY_CONFIG, true);
        for (int i = 0; i < portalParamCount; i++) {
            String storedVal = persistentStore.getString(params[i].key, "");
            strcpy(params[i].value, storedVal.c_str());

            params[i].paramInstance = new WiFiManagerParameter(
                params[i].key, params[i].label, params[i].value, params[i].length
            );
            onAddParameter(params[i].paramInstance);
        }
        persistentStore.end();
    }
}

#endif
