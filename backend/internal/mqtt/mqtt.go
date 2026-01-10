package mqtt

import (
	"log"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/listeners"
)

const port = ":1883"

func Run() *mqtt.Server {
	server := mqtt.New(&mqtt.Options{InlineClient: true})
	addHooks(server)

	tcp := listeners.NewTCP(listeners.Config{ID: "t1", Address: port})
	if err := server.AddListener(tcp); err != nil {
		log.Fatal(err)
	}

	go func() {
		if err := server.Serve(); err != nil {
			log.Fatal(err)
		}
	}()

	if err := subscribeToDeviceTopic(server); err != nil {
		log.Fatal(err)
	}

	return server
}
