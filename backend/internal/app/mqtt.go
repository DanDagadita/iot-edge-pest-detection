package app

import (
	"log"
	"os"
	"os/signal"
	"syscall"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/listeners"
)

const port = ":1883"

func RunMQTTServer() {
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

	addHooks(server)

	tcp := listeners.NewTCP(listeners.Config{ID: "t1", Address: port})
	err := server.AddListener(tcp)
	if err != nil {
		log.Fatal(err)
	}

	go func() {
		err := server.Serve()
		if err != nil {
			log.Fatal(err)
		}
	}()

	err = subscribeToDeviceTopic(server)
	if err != nil {
		log.Fatal(err)
	}

	<-done
	_ = server.Close()
}
