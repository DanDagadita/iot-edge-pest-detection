#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

namespace MqttHandler {
    constexpr int LOOP_LIMIT_MS = 70;
    constexpr int CLIENT_TIMEOUT_S = 2;
    constexpr int JSON_BUFFER_SIZE = 256;
    constexpr int TOPIC_BUFFER_SIZE = 64;
    constexpr int MAC_BUFFER_SIZE = 18;

    char deviceMac[MAC_BUFFER_SIZE];
    bool isMqttConnected = false;

    WiFiClient networkClient;
    PubSubClient mqttMessenger(networkClient);

    void (*onConfigReceived)(JsonDocument doc) = nullptr;

    void onMessageReceived(const char* topic, const byte* payload, const unsigned int length) {
        JsonDocument doc;
        deserializeJson(doc, payload, length);
        Hardware::log("[MQTT] Received message");

        if (onConfigReceived != nullptr) {
            onConfigReceived(doc);
        }
    }

    void subscribeToTopic(const char* topic) {
        mqttMessenger.subscribe(topic);
        Hardware::log("[MQTT] Subscribed to topic %s", topic);
    }

    void publishToTopic(const char* topic, const JsonDocument& doc) {
        char buffer[JSON_BUFFER_SIZE];
        serializeJson(doc, buffer);
        mqttMessenger.publish(topic, buffer);
        Hardware::log("[MQTT] Published to topic %s", topic);
    }

    void subscribeToDeviceTopic() {
        char topic[TOPIC_BUFFER_SIZE];
        snprintf(topic, sizeof(topic), "device/config/%s", deviceMac);
        subscribeToTopic(topic);
    }

    void publishConfig() {
        JsonDocument doc;
        doc["mac"] = deviceMac;
        doc["token"] = Config::settings.userToken;
        publishToTopic("device/pair", doc);
    }

    void handleDetection(const float probability, const float intensity) {
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

    void attemptReconnect() {
        mqttMessenger.setServer(Config::settings.mqttServer, atoi(Config::settings.mqttPort));
        Hardware::log("[MQTT] Connecting");
        Hardware::printToLCD("MQTT Connecting");

        if (mqttMessenger.connect(deviceMac, Config::settings.mqttUsername, Config::settings.mqttPassword)) {
            isMqttConnected = true;
            Hardware::log("[MQTT] Connected!");
            Hardware::printToLCD("MQTT Connected!");
            subscribeToDeviceTopic();
            publishConfig();
        } else {
            isMqttConnected = false;
            Hardware::log("[MQTT] Connection failed, client state: %d", mqttMessenger.state());
            Hardware::printToLCD("MQTT Connection failed state %d", mqttMessenger.state());
            delay(CLIENT_TIMEOUT_S * 1000);
        }
    }

    void handleConfigChange() {
        if (mqttMessenger.connected()) {
            mqttMessenger.disconnect();
        }
        attemptReconnect();
    }

    void setup() {
        strncpy(deviceMac, WiFi.macAddress().c_str(), sizeof(deviceMac));
        mqttMessenger.setCallback(onMessageReceived);
        mqttMessenger.setSocketTimeout(CLIENT_TIMEOUT_S);
        networkClient.setTimeout(CLIENT_TIMEOUT_S);
    }

    void loop() {
        static unsigned long lastLoop = 0;
        if (lastLoop == 0 || millis() - lastLoop > LOOP_LIMIT_MS) {
            if (!mqttMessenger.connected()) {
                attemptReconnect();
            }
            mqttMessenger.loop();
            lastLoop = millis();
        }
    }
}

#endif
