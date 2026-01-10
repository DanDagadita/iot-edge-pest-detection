package app

import (
	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/hooks/auth"
)

func addHooks(server *mqtt.Server) {
	// for now, allow all connections
	_ = server.AddHook(new(auth.AllowHook), nil)
}
