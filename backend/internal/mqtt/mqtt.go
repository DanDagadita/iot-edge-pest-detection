package mqtt

import (
	"log"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/listeners"
)

const tcpPort = ":1883"
const wsPort = ":1882"

func Run() *mqtt.Server {
	server := mqtt.New(&mqtt.Options{InlineClient: true})
	addHooks(server)

	tcp := listeners.NewTCP(listeners.Config{ID: "t1", Address: tcpPort})
	if err := server.AddListener(tcp); err != nil {
		log.Fatal(err)
	}

	ws := listeners.NewWebsocket(listeners.Config{ID: "w1", Address: wsPort})
	if err := server.AddListener(ws); err != nil {
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
