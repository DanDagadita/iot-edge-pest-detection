package handlers

import (
	"encoding/json"
	"fmt"
	"pest-detector/internal/models"
)

type PairingMessage struct {
	MAC string `json:"mac"`
}

type TelemetryMessage struct {
	Probability float32 `json:"probability"`
	Intensity   float32 `json:"intensity"`
}

const DefaultThreshold = 0.9

type MQTTHandler struct {
	dbHandler *DBHandler
}

func NewMQTTHandler(dbHandler *DBHandler) *MQTTHandler {
	return &MQTTHandler{
		dbHandler: dbHandler,
	}
}

func (h *MQTTHandler) HandlePair(payload []byte) (*models.Device, error) {
	var pairing PairingMessage
	err := json.Unmarshal(payload, &pairing)
	errMsg := ""
	if err != nil {
		errMsg = err.Error()
	}

	h.dbHandler.logPairing(pairing.MAC, string(payload), err == nil, errMsg)

	if err != nil {
		return nil, fmt.Errorf("pairing unmarshal error: %w", err)
	}

	device, err := h.dbHandler.createOrUpdateDevice(pairing.MAC, DefaultThreshold)
	if err != nil {
		h.dbHandler.logPairing(pairing.MAC, string(payload), false, fmt.Sprintf("database error: %v", err))
		return nil, fmt.Errorf("database error during pairing: %w", err)
	}

	return device, nil
}

func (h *MQTTHandler) HandleTelemetry(payload []byte, mac string) error {
	var telemetry TelemetryMessage
	err := json.Unmarshal(payload, &telemetry)
	if err != nil {
		return fmt.Errorf("telemetry unmarshal error: %w", err)
	}

	err = h.dbHandler.UpdateDevice(mac, true, true)
	if err != nil {
		return fmt.Errorf("device status update error: %w", err)
	}

	err = h.dbHandler.AddDetectionEvent(mac, telemetry.Probability, telemetry.Intensity)
	if err != nil {
		return fmt.Errorf("add detection event error: %w", err)
	}

	return nil
}
