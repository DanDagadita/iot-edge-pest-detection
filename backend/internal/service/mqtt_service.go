package service

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	mqtt "github.com/mochi-mqtt/server/v2"
	"github.com/mochi-mqtt/server/v2/packets"

	"pest-detector/internal/database"
	"pest-detector/internal/handlers"
	"pest-detector/internal/models"
)

type MQTTService struct {
	dbHandler   *handlers.DBHandler
	mqttHandler *handlers.MQTTHandler
	mqttServer  *mqtt.Server
}

func NewMQTTService(mqttServer *mqtt.Server) *MQTTService {
	db, err := database.InitializeDatabase()
	if err != nil {
		log.Fatal(err)
	}
	dbHandler := handlers.NewDBHandler(db)
	mqttHandler := handlers.NewMQTTHandler(dbHandler)
	return &MQTTService{dbHandler: dbHandler, mqttHandler: mqttHandler, mqttServer: mqttServer}
}

func (s *MQTTService) GetMQTTServer() *mqtt.Server {
	return s.mqttServer
}

func (s *MQTTService) GetDBHandler() *handlers.DBHandler {
	return s.dbHandler
}

func (s *MQTTService) SubscribeToDeviceTopic() error {
	err := s.mqttServer.Subscribe("device/#", 1, func(cl *mqtt.Client, sub packets.Subscription, pk packets.Packet) {
		payload := pk.Payload
		topic := pk.TopicName

		if strings.Contains(topic, "device/pair") {
			s.handlePair(payload)
		}

		if strings.Contains(topic, "device/telemetry/") {
			s.handleTelemetry(payload, topic)
		}
	})
	return err
}

func (s *MQTTService) StartCleanupRoutine() {
	ticker := time.NewTicker(24 * time.Hour)
	defer ticker.Stop()

	if err := s.dbHandler.CleanupOldData(30); err != nil {
		log.Printf("Initial cleanup error: %v", err)
	}

	for range ticker.C {
		log.Println("Running database cleanup...")
		if err := s.dbHandler.CleanupOldData(30); err != nil {
			log.Printf("Cleanup error: %v", err)
		}
		log.Println("Database cleanup completed")
	}
}

func (s *MQTTService) HandleShutdown() {
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	go func() {
		<-sigChan
		log.Println("Shutdown signal received...")

		s.mqttServer.Close()

		if err := s.dbHandler.CleanupOldData(30); err != nil {
			log.Printf("Final cleanup error: %v", err)
		}

		log.Println("Shutdown complete")
		os.Exit(0)
	}()
}

func (s *MQTTService) handlePair(payload []byte) {
	device, err := s.mqttHandler.HandlePair(payload)
	if err != nil {
		s.logError(err, "enhanced pairing error", payload, "")
		return
	}

	var pairingData handlers.PairingMessage
	if err := json.Unmarshal(payload, &pairingData); err == nil && pairingData.MAC != "" {
		config := models.Config{Threshold: device.Threshold}
		s.publishConfig(pairingData.MAC, config)
	}
}

func (s *MQTTService) handleTelemetry(payload []byte, topic string) {
	parts := strings.Split(topic, "/")
	mac := parts[len(parts)-1]

	err := s.mqttHandler.HandleTelemetry(payload, mac)
	if err != nil {
		s.logError(err, "enhanced telemetry error", payload, mac)
		return
	}

	s.logInfo("telemetry success", payload, mac)
}

func (s *MQTTService) publishConfig(mac string, config models.Config) {
	payload, err := json.Marshal(config)
	if err != nil {
		s.logError(err, "config marshal error", payload, mac)
		return
	}
	configTopic := fmt.Sprintf("device/config/%s", mac)
	err = s.mqttServer.Publish(configTopic, payload, false, 0)
	if err != nil {
		s.logError(err, "config publish error", payload, mac)
		return
	}
	s.logInfo("config publish success", payload, mac)
}

func (s *MQTTService) logError(err error, message string, payload []byte, mac string) {
	s.mqttServer.Log.Error(message, "error", err.Error(), "payload", string(payload), "mac", mac)
}

func (s *MQTTService) logInfo(message string, payload []byte, mac string) {
	s.mqttServer.Log.Info(message, "payload", payload, "mac", mac)
}
