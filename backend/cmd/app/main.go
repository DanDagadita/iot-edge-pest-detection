package main

import (
	"pest-detection/internal/http"
	"pest-detection/internal/mqtt"
)

func main() {
	mqtt.Run()
	http.Run()
}
