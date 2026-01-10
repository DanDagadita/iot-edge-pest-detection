package app

import (
	"encoding/json"
	"fmt"
	"strings"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/packets"
)

type Pairing struct {
	MAC       string `json:"mac"`
	UserToken string `json:"token"`
}

type Config struct {
	Threshold float32 `json:"threshold"`
	Window    uint32  `json:"window"`
}

type Device struct {
	MAC       string  `json:"mac"`
	UserToken string  `json:"token"`
	Threshold float32 `json:"threshold"`
	Window    uint32  `json:"window"`
}

type Telemetry struct {
	Probability float32 `json:"probability"`
	Intensity   float32 `json:"intensity"`
}

const defaultThreshold = 0.9
const defaultWindow = 10

var devices = make(map[string]Device)

func handlePair(server *mqtt.Server, payload []byte) {
	var data Pairing
	err := json.Unmarshal(payload, &data)
	if err != nil {
		logError(server, err, "pairing unmarshal error", payload, "")
		return
	}
	config := Config{Threshold: defaultThreshold, Window: defaultWindow}
	devices[data.MAC] = Device{
		MAC:       data.MAC,
		UserToken: data.UserToken,
		Threshold: config.Threshold,
		Window:    config.Window,
	}
	logInfo(server, "pairing success", payload, data.MAC)
	publishConfig(server, data.MAC, config)
}

func publishConfig(server *mqtt.Server, mac string, config Config) {
	payload, err := json.Marshal(config)
	if err != nil {
		logError(server, err, "config marshal error", payload, mac)
		return
	}
	configTopic := fmt.Sprintf("device/config/%s", mac)
	err = server.Publish(configTopic, payload, false, 0)
	if err != nil {
		logError(server, err, "config publish error", payload, mac)
		return
	}
	logInfo(server, "config publish success", payload, mac)
}

func handleTelemetry(server *mqtt.Server, payload []byte, topic string) {
	parts := strings.Split(topic, "/")
	mac := parts[len(parts)-1]
	var telemetry Telemetry
	err := json.Unmarshal(payload, &telemetry)
	if err != nil {
		logError(server, err, "telemetry unmarshal error", payload, mac)
		return
	}
	logInfo(server, "telemetry success", payload, mac)
}

func subscribeToDeviceTopic(server *mqtt.Server) error {
	err := server.Subscribe("device/#", 1, func(cl *mqtt.Client, sub packets.Subscription, pk packets.Packet) {
		payload := pk.Payload
		topic := pk.TopicName

		if strings.Contains(topic, "device/pair") {
			handlePair(server, payload)
		}

		if strings.Contains(topic, "device/telemetry/") {
			handleTelemetry(server, payload, topic)
		}
	})
	return err
}

func logError(server *mqtt.Server, err error, message string, payload []byte, mac string) {
	server.Log.Error(message, "error", err.Error(), "payload", string(payload), "mac", mac)
}

func logInfo(server *mqtt.Server, message string, payload []byte, mac string) {
	server.Log.Info(message, "payload", payload, "mac", mac)
}
