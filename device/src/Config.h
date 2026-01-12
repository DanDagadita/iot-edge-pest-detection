#ifndef CONFIG_H
#define CONFIG_H

class WiFiManagerParameter;

namespace Config {
    struct configSettings {
        char mqttServer[40];
        char mqttPort[6];
        char mqttUsername[40];
        char mqttPassword[40];
        char userToken[64];
    };

    extern void (*OnConfigChange)();
    extern void (*OnAddParameter)(WiFiManagerParameter*);

    configSettings GetSettings();

    void HandleParamsSave();
    void Setup();
}

#endif
