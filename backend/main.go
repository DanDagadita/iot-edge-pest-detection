package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/hooks/auth"
	"github.com/mochi-mqtt/server/v2/listeners"
	"github.com/mochi-mqtt/server/v2/packets"
)

type Device struct {
	MAC       string  `json:"mac"`
	UserToken string  `json:"token"`
	Threshold float32 `json:"threshold"`
}

var devices = make(map[string]Device)

func main() {
	sigs := make(chan os.Signal, 1)
	done := make(chan bool, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		<-sigs
		done <- true
	}()

	server := mqtt.New(&mqtt.Options{
		InlineClient: true,
	})

	// for now, allow all connections
	_ = server.AddHook(new(auth.AllowHook), nil)

	tcp := listeners.NewTCP(listeners.Config{ID: "t1", Address: ":1883"})
	err := server.AddListener(tcp)
	if err != nil {
		log.Fatal(err)
	}

	callbackFn := func(cl *mqtt.Client, sub packets.Subscription, pk packets.Packet) {
		payload := pk.Payload
		topic := pk.TopicName

		server.Log.Info("Inbound Message", "topic", topic, "payload", string(payload))

		if topic == "device/pair" {
			var data Device
			if err := json.Unmarshal(payload, &data); err == nil {
				data.Threshold = 0.9 // Set default threshold
				devices[data.MAC] = data
				fmt.Printf("PAIRED: Device [%s] with Token [%s]\n", data.MAC, data.UserToken)

				// send initial config back to device
				configTopic := fmt.Sprintf("device/cmd/%s", data.MAC)
				configPayload := `{"threshold": 0.9, "window": 10}`
				server.Publish(configTopic, []byte(configPayload), false, 0)
			}
		}

		// handling telemetry (device/telemetry/*)
		// example: device/telemetry/AA:BB:CC:DD:EE:FF
		if len(topic) > 15 && topic[:15] == "device/telemetry/" {
			var telemetry struct {
				Prob      float32 `json:"prob"`
				Intensity float32 `json:"intensity"`
			}
			if err := json.Unmarshal(payload, &telemetry); err == nil {
				mac := topic[15:]
				fmt.Printf("DETECTION from [%s]: Prob: %.2f, Intensity: %.2f\n", mac, telemetry.Prob, telemetry.Intensity)
			}
		}
	}

	err = server.Subscribe("device/#", 1, callbackFn)
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		err := server.Serve()
		if err != nil {
			log.Fatal(err)
		}
	}()

	<-done
	server.Close()
}
