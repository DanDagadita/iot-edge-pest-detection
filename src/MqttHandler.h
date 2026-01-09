#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

namespace MqttHandler {
    constexpr int CLIENT_TIMEOUT_SECONDS = 2;
    constexpr int RECONNECT_TIMEOUT_MILISECONDS = 10 * 1000;
    constexpr int JSON_BUFFER_SIZE = 256;
    constexpr int TOPIC_BUFFER_SIZE = 64;
    constexpr int MAC_BUFFER_SIZE = 18;

    WiFiClient networkClient;
    PubSubClient mqttMessenger(networkClient);

    char deviceMac[MAC_BUFFER_SIZE];
    bool isMqttConnected = false;
    unsigned long lastReconnectTimestamp = 0;

    void (*onCommandReceived)(JsonDocument doc) = nullptr;

    void onMessageReceived(const char* topic, const byte* payload, const unsigned int length) {
        JsonDocument doc;
        deserializeJson(doc, payload, length);
        Hardware::log("[MQTT] Received message");

        if (onCommandReceived != nullptr) {
            onCommandReceived(doc);
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

    void sendDeviceHandshake() {
        char topic[TOPIC_BUFFER_SIZE];
        snprintf(topic, sizeof(topic), "device/cmd/%s", deviceMac);
        subscribeToTopic(topic);

        JsonDocument doc;
        doc["mac"] = deviceMac;
        doc["token"] = Config::settings.userToken;
        publishToTopic("device/pair", doc);
    }

    void handleDetection(const float prob, const float intensity) {
        if (!mqttMessenger.connected()) {
            return;
        }

        JsonDocument doc;
        doc["prob"] = prob;
        doc["intensity"] = intensity;
        char topic[TOPIC_BUFFER_SIZE];
        snprintf(topic, sizeof(topic), "device/telemetry/%s", deviceMac);
        publishToTopic(topic, doc);
    }

    void attemptReconnect(const bool skipTimer = false) {
        if (!skipTimer && lastReconnectTimestamp != 0 && millis() - lastReconnectTimestamp < RECONNECT_TIMEOUT_MILISECONDS) {
            return;
        }
        lastReconnectTimestamp = millis();

        mqttMessenger.setServer(Config::settings.mqttServer, atoi(Config::settings.mqttPort));
        Hardware::log("[MQTT] Connecting...");

        if (mqttMessenger.connect(deviceMac, Config::settings.mqttUsername, Config::settings.mqttPassword)) {
            isMqttConnected = true;
            Hardware::log("[MQTT] Connected");
            sendDeviceHandshake();
        } else {
            isMqttConnected = false;
            Hardware::log("[MQTT] Connection failed, client state: %d", mqttMessenger.state());
        }
    }

    void handleConfigChange() {
        if (mqttMessenger.connected()) {
            mqttMessenger.disconnect();
        }
        attemptReconnect(true);
    }

    void setup() {
        strncpy(deviceMac, WiFi.macAddress().c_str(), sizeof(deviceMac));
        mqttMessenger.setCallback(onMessageReceived);
        networkClient.setTimeout(CLIENT_TIMEOUT_SECONDS);
    }

    void loop() {
        if (!mqttMessenger.connected()) {
            attemptReconnect();
            return;
        }
        mqttMessenger.loop();
    }
}

#endif
