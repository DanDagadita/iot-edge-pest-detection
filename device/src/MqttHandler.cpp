#include "MqttHandler.h"
#include "Hardware.h"
#include "Config.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

namespace {
    constexpr int LOOP_LIMIT_MS = 70;
    constexpr int LOOP_LIMIT_RECONNECT_MS = 2000;
    constexpr int CLIENT_TIMEOUT_S = 2;
    constexpr int JSON_BUFFER_SIZE = 256;
    constexpr int TOPIC_BUFFER_SIZE = 64;
    constexpr int MAC_BUFFER_SIZE = 18;

    char deviceMac[MAC_BUFFER_SIZE];
    bool isMqttConnected = false;

    WiFiClient networkClient;
    PubSubClient mqttMessenger(networkClient);

    void onMessageReceived(const char* topic, const byte* payload, const unsigned int length) {
        JsonDocument doc;
        deserializeJson(doc, payload, length);
        Hardware::Log("[MQTT] Received message");
        MqttHandler::OnConfigReceived(doc);
    }

    void subscribeToTopic(const char* topic) {
        mqttMessenger.subscribe(topic);
        Hardware::Log("[MQTT] Subscribed to topic %s", topic);
    }

    void publishToTopic(const char* topic, const JsonDocument& doc) {
        char buffer[JSON_BUFFER_SIZE];
        serializeJson(doc, buffer);
        mqttMessenger.publish(topic, buffer);
        Hardware::Log("[MQTT] Published to topic %s", topic);
    }

    void subscribeToDeviceTopic() {
        char topic[TOPIC_BUFFER_SIZE];
        snprintf(topic, sizeof(topic), "device/config/%s", deviceMac);
        subscribeToTopic(topic);
    }

    void publishConfig() {
        JsonDocument doc;
        doc["mac"] = deviceMac;
        doc["token"] = Config::GetSettings().userToken;
        publishToTopic("device/pair", doc);
    }

    void attemptReconnect() {
        mqttMessenger.setServer(Config::GetSettings().mqttServer, atoi(Config::GetSettings().mqttPort));
        Hardware::Log("[MQTT] Connecting");
        Hardware::PrintToLCD("MQTT Connecting");

        if (mqttMessenger.connect(deviceMac, Config::GetSettings().mqttUsername, Config::GetSettings().mqttPassword)) {
            isMqttConnected = true;
            Hardware::Log("[MQTT] Connected!");
            Hardware::PrintToLCD("MQTT Connected!");
            subscribeToDeviceTopic();
            publishConfig();
        } else {
            isMqttConnected = false;
            Hardware::Log("[MQTT] Connection failed, client state: %d", mqttMessenger.state());
            Hardware::PrintToLCD("MQTT Connection failed state %d", mqttMessenger.state());
        }
    }
}

namespace MqttHandler {
    void (*OnConfigReceived)(const JsonDocument&) = nullptr;

    bool GetIsMqttConnected() {
        return isMqttConnected;
    }

    void HandleDetection(const float probability, const float intensity) {
        if (!isMqttConnected) {
            return;
        }

        JsonDocument doc;
        doc["probability"] = probability;
        doc["intensity"] = intensity;
        char topic[TOPIC_BUFFER_SIZE];
        snprintf(topic, sizeof(topic), "device/telemetry/%s", deviceMac);
        publishToTopic(topic, doc);
    }

    void HandleConfigChange() {
        if (mqttMessenger.connected()) {
            mqttMessenger.disconnect();
        }
        attemptReconnect();
    }

    void Setup() {
        strncpy(deviceMac, WiFi.macAddress().c_str(), sizeof(deviceMac));
        mqttMessenger.setCallback(onMessageReceived);
        mqttMessenger.setSocketTimeout(CLIENT_TIMEOUT_S);
        networkClient.setTimeout(CLIENT_TIMEOUT_S);
    }

    void Loop() {
        static unsigned long lastLoop = 0;
        if (lastLoop == 0 || millis() - lastLoop > LOOP_LIMIT_MS) {
            mqttMessenger.loop();
            lastLoop = millis();
        }
 
        static unsigned long lastLoopReconnect = 0;
        if (lastLoopReconnect == 0 || millis() - lastLoopReconnect > LOOP_LIMIT_RECONNECT_MS) {
            if (!mqttMessenger.connected()) {
                attemptReconnect();
            }
            lastLoopReconnect = millis();
        }
    }
}
