package http

import (
	"errors"
	"log"
	"log/slog"
	"net/http"
	"pest-detector/internal/service"

	"github.com/labstack/echo/v4"
	"github.com/labstack/echo/v4/middleware"
)

func Run(mqttService *service.MQTTService) {
	e := echo.New()

	e.Use(middleware.RequestLogger())
	e.Use(middleware.Recover())
	e.Use(middleware.CORS())

	httpService := service.NewHTTPService(mqttService)

	e.GET("/devices", httpService.GetDevices)
	e.GET("/devices/:mac", httpService.GetDevice)
	e.GET("/devices/:mac/detections", httpService.GetDetections)
	e.PUT("/devices/:mac/config", httpService.UpdateConfig)

	if err := e.Start(":8080"); err != nil && !errors.Is(err, http.ErrServerClosed) {
		slog.Error("failed to start server", "error", err)
		log.Fatal(err)
	}
}
