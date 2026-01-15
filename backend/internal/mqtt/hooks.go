package mqtt

import (
	"log"
	"pest-detector/internal/service"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/hooks/auth"
)

type DisconnectHook struct {
	mqtt.HookBase
	service *service.MQTTService
}

func (h *DisconnectHook) ID() string {
	return "disconnect-logger"
}

func (h *DisconnectHook) Provides(b byte) bool {
	return b == mqtt.OnDisconnect
}

func (h *DisconnectHook) OnDisconnect(cl *mqtt.Client, err error, expire bool) {
	h.service.GetDBHandler().UpdateDevice(cl.ID, false, false)
}

func addHooks(service *service.MQTTService) {
	if err := service.GetMQTTServer().AddHook(new(auth.AllowHook), nil); err != nil {
		log.Fatal(err)
	}

	if err := service.GetMQTTServer().AddHook(&DisconnectHook{service: service}, nil); err != nil {
		log.Fatal(err)
	}
}
