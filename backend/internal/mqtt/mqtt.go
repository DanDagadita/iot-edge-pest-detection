package mqtt

import (
	"log"
	"pest-detector/internal/service"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/listeners"
)

const tcpPort = ":1883"
const wsPort = ":1882"

func Run() *service.MQTTService {
	server := mqtt.New(&mqtt.Options{InlineClient: true})
	mqttService := service.NewMQTTService(server)

	addHooks(mqttService)

	tcp := listeners.NewTCP(listeners.Config{ID: "t1", Address: tcpPort})
	if err := server.AddListener(tcp); err != nil {
		log.Fatal(err)
	}

	ws := listeners.NewWebsocket(listeners.Config{ID: "w1", Address: wsPort})
	if err := server.AddListener(ws); err != nil {
		log.Fatal(err)
	}

	go mqttService.StartCleanupRoutine()
	go mqttService.HandleShutdown()

	go func() {
		if err := server.Serve(); err != nil {
			log.Fatal(err)
		}
	}()

	if err := mqttService.SubscribeToDeviceTopic(); err != nil {
		log.Fatal(err)
	}
	log.Println("MQTT server started")
	return mqttService
}
