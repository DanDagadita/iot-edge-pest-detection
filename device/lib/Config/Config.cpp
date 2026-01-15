#include "Config.h"
#include "Hardware.h"
#include <WiFiManager.h>
#include <Preferences.h>
#include <Arduino.h>

namespace {
    constexpr const char* KEY_CONFIG = "config";

    Config::configSettings settings;

    struct param {
        const char* key;
        const char* label;
        int length;
        char* value;
        WiFiManagerParameter* paramInstance;
    };

    param params[] = {
        {"mqtt_server", "MQTT Server", 40, settings.mqttServer},
        {"mqtt_port", "MQTT Port", 6, settings.mqttPort},
        {"mqtt_username", "MQTT Username", 40, settings.mqttUsername},
        {"mqtt_password", "MQTT Password", 40, settings.mqttPassword},
    };
    const int portalParamCount = sizeof(params) / sizeof(params[0]);
    Preferences persistentStore;
}

void (*Config::OnConfigChange)() = nullptr;
void (*Config::OnAddParameter)(WiFiManagerParameter*) = nullptr;

Config::configSettings Config::GetSettings() {
    return settings;
}

void Config::HandleParamsSave() {
    persistentStore.begin(KEY_CONFIG, false);
    for (int i = 0; i < portalParamCount; i++) {
        strcpy(params[i].value, params[i].paramInstance->getValue());
        persistentStore.putString(params[i].key, params[i].value);
    }
    persistentStore.end();

    Hardware::Log("[Config] Settings saved");
    OnConfigChange();
}

void Config::Setup() {
    persistentStore.begin(KEY_CONFIG, true);
    for (int i = 0; i < portalParamCount; i++) {
        String storedVal = persistentStore.getString(params[i].key, "");
        strcpy(params[i].value, storedVal.c_str());

        params[i].paramInstance = new WiFiManagerParameter(
            params[i].key, params[i].label, params[i].value, params[i].length
        );
        OnAddParameter(params[i].paramInstance);
    }
    persistentStore.end();
}
