package main

import (
	"pest-detector/internal/http"
	"pest-detector/internal/mqtt"
)

func main() {
	mqttServer := mqtt.Run()
	http.Run(mqttServer)
}
