#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

namespace MqttHandler {
    unsigned long lastReconnectAttempt = 0;
    WiFiClient espClient;
    PubSubClient client(espClient);
    String macAddr;
    bool connection_success = false;
    void (*onCallback)(JsonDocument doc) = nullptr;

    void callback(char* topic, byte* payload, unsigned int length) {
        JsonDocument doc;
        deserializeJson(doc, payload, length);

        if (onCallback != nullptr) {
            onCallback(doc);
        }
    }

    void setup() {
        macAddr = WiFi.macAddress();
        client.setCallback(callback);
        espClient.setTimeout(2);
    }

    void sendPairing() {
        JsonDocument doc;
        doc["mac"] = macAddr;
        doc["token"] = Config::get("user_token");

        char buffer[256];
        serializeJson(doc, buffer);
        client.publish("pest/pair", buffer);
        Serial.println("[MQTT] Sent Pairing Message");

        String cmdTopic = "pest/cmd/" + macAddr;
        client.subscribe(cmdTopic.c_str());
    }

    void reconnect(bool skipTimer = false) {
        if (!skipTimer && millis() - lastReconnectAttempt < 10000) {
            return;
        }
        lastReconnectAttempt = millis();

        client.setServer(Config::get("mqtt_server"), atoi(Config::get("mqtt_port")));
        Serial.print("[MQTT] Connecting...");
        if (client.connect(macAddr.c_str(), Config::get("mqtt_username"), Config::get("mqtt_password"))) {
            connection_success = true;
            Serial.println("connected");
            sendPairing();
        } else {
            connection_success = false;
            Serial.print("failed, rc=");
            Serial.print(client.state());
        }
    }

    void handleDetection(float prob, float intensity) {
        if (!client.connected()) {
            return;
        }

        JsonDocument doc;
        doc["prob"] = prob;
        doc["intensity"] = intensity;
        
        char buffer[256];
        serializeJson(doc, buffer);
        String topic = "pest/telemetry/" + macAddr;
        client.publish(topic.c_str(), buffer);
    }

    void handleConfigChange() {
        if (client.connected()) {
            client.disconnect();
        }
        reconnect(true);
    }

    void loop() {
        if (!client.connected()) {
            reconnect();
            return;
        }
        client.loop();
    }
}

#endif
