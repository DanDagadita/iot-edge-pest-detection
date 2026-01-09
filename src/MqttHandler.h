#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

namespace MqttHandler {
    WiFiClient networkClient;
    PubSubClient mqttMessenger(networkClient);

    String deviceMac;
    bool isMqttConnected = false;
    unsigned long lastReconnectTimestamp = 0;

    void (*onCommandReceived)(JsonDocument doc) = nullptr;

    void onMessageReceived(char* topic, byte* payload, unsigned int length) {
        JsonDocument doc;
        deserializeJson(doc, payload, length);
        Hardware::log("[MQTT] Received message");

        if (onCommandReceived != nullptr) {
            onCommandReceived(doc);
        }
    }

    void sendDeviceHandshake() {
        JsonDocument doc;
        doc["mac"] = deviceMac;
        doc["token"] = Config::getParamValue("user_token");

        char buffer[256];
        serializeJson(doc, buffer);
        mqttMessenger.publish("device/pair", buffer);

        Hardware::log("[MQTT] Sent pairing message");

        String cmdTopic = "device/cmd/" + deviceMac;
        mqttMessenger.subscribe(cmdTopic.c_str());
    }

    void setup() {
        deviceMac = WiFi.macAddress();
        mqttMessenger.setCallback(onMessageReceived);
        networkClient.setTimeout(2);
    }

    void attemptReconnect(bool skipTimer = false) {
        if (!skipTimer && millis() - lastReconnectTimestamp < 10000) {
            return;
        }
        lastReconnectTimestamp = millis();

        mqttMessenger.setServer(Config::getParamValue("mqtt_server"), atoi(Config::getParamValue("mqtt_port")));
        Hardware::log("[MQTT] Connecting...");

        if (mqttMessenger.connect(deviceMac.c_str(), Config::getParamValue("mqtt_username"), Config::getParamValue("mqtt_password"))) {
            isMqttConnected = true;
            Hardware::log("[MQTT] connected");
            sendDeviceHandshake();
        } else {
            isMqttConnected = false;
            Hardware::log("[MQTT] failed, rc=%d", mqttMessenger.state());
        }
    }

    void handleDetection(float prob, float intensity) {
        if (!mqttMessenger.connected()) {
            return;
        }

        JsonDocument doc;
        doc["prob"] = prob;
        doc["intensity"] = intensity;

        char buffer[256];
        serializeJson(doc, buffer);
        String topic = "device/telemetry/" + deviceMac;
        mqttMessenger.publish(topic.c_str(), buffer);
    }

    void handleConfigChange() {
        if (mqttMessenger.connected()) {
            mqttMessenger.disconnect();
        }
        attemptReconnect(true);
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
