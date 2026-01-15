package models

type DeviceSummary struct {
	Device
	Detections24h int `json:"detections24h"`
}

type Config struct {
	Threshold float32 `json:"threshold"`
}
