package http

import (
	"errors"
	"log"
	"log/slog"
	"net/http"
	"pest-detection/internal/mqtt"

	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
)

func Run() {
	e := echo.New()

	e.Use(middleware.RequestLogger())
	e.Use(middleware.Recover())

	e.GET("/", hello)

	if err := e.Start(":8080"); err != nil && !errors.Is(err, http.ErrServerClosed) {
		slog.Error("failed to start server", "error", err)
		log.Fatal(err)
	}
}

func hello(c echo.Context) error {
	mqtt.DeviceMutex.RLock()
	defer mqtt.DeviceMutex.RUnlock()
	return c.JSON(http.StatusOK, mqtt.Devices)
}
