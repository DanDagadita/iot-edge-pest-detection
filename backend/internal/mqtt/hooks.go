package mqtt

import (
	"log"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/hooks/auth"
)

type DisconnectHook struct {
	mqtt.HookBase
}

func (h *DisconnectHook) ID() string {
	return "disconnect-logger"
}

func (h *DisconnectHook) Provides(b byte) bool {
	return b == mqtt.OnDisconnect
}

func (h *DisconnectHook) OnDisconnect(cl *mqtt.Client, err error, expire bool) {
	DeviceMutex.Lock()
	delete(Devices, cl.ID)
	DeviceMutex.Unlock()
}

func addHooks(server *mqtt.Server) {
	// for now, allow all connections
	if err := server.AddHook(new(auth.AllowHook), nil); err != nil {
		log.Fatal(err)
	}

	if err := server.AddHook(new(DisconnectHook), nil); err != nil {
		log.Fatal(err)
	}
}
