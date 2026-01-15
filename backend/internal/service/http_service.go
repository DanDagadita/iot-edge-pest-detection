package service

import (
	"log"
	"net/http"
	"pest-detector/internal/database"
	"pest-detector/internal/handlers"
	"pest-detector/internal/models"

	"github.com/labstack/echo/v4"
)

type HTTPService struct {
	dbHandler   *handlers.DBHandler
	mqttService *MQTTService
}

func NewHTTPService(mqttService *MQTTService) *HTTPService {
	db, err := database.InitializeDatabase()
	if err != nil {
		log.Fatal(err)
	}
	dbHandler := handlers.NewDBHandler(db)
	return &HTTPService{dbHandler: dbHandler, mqttService: mqttService}
}

func (s *HTTPService) GetDevices(c echo.Context) error {
	devices, err := s.dbHandler.GetDevices()
	if err != nil {
		return c.JSON(http.StatusInternalServerError, map[string]string{
			"error": "Failed to get devices: " + err.Error(),
		})
	}

	return c.JSON(http.StatusOK, devices)
}

func (s *HTTPService) GetDevice(c echo.Context) error {
	mac := c.Param("mac")
	if mac == "" {
		return c.JSON(http.StatusBadRequest, map[string]string{
			"error": "MAC address is required",
		})
	}

	device, err := s.dbHandler.GetDevice(mac)
	if err != nil {
		return c.JSON(http.StatusNotFound, map[string]string{
			"error": "Device not found: " + err.Error(),
		})
	}

	return c.JSON(http.StatusOK, device)
}

func (s *HTTPService) GetDetections(c echo.Context) error {
	mac := c.Param("mac")
	if mac == "" {
		return c.JSON(http.StatusBadRequest, map[string]string{
			"error": "MAC address is required",
		})
	}

	detections, err := s.dbHandler.GetDetections(mac)
	if err != nil {
		return c.JSON(http.StatusInternalServerError, map[string]string{
			"error": "Failed to get detections: " + err.Error(),
		})
	}

	return c.JSON(http.StatusOK, detections)
}

func (s *HTTPService) UpdateConfig(c echo.Context) error {
	mac := c.Param("mac")
	if mac == "" {
		return c.JSON(http.StatusBadRequest, map[string]string{
			"error": "MAC address is required",
		})
	}

	var config models.Config
	err := c.Bind(&config)
	if err != nil {
		return c.JSON(http.StatusBadRequest, map[string]string{
			"error": "Invalid body: " + err.Error(),
		})
	}

	err = s.dbHandler.UpdateConfig(mac, config.Threshold)
	if err != nil {
		return c.JSON(http.StatusInternalServerError, map[string]string{
			"error": "Failed to update config: " + err.Error(),
		})
	}

	s.mqttService.publishConfig(mac, config)

	return c.JSON(http.StatusOK, config)
}
