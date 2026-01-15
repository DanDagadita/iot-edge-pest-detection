package models

import (
	"time"
)

type Model struct {
	ID        uint      `gorm:"primarykey" json:"id"`
	CreatedAt time.Time `json:"createdAt"`
	UpdatedAt time.Time `json:"updatedAt"`
}

type Device struct {
	Model
	MAC             string  `gorm:"uniqueIndex;size:17" json:"mac"`
	Threshold       float32 `gorm:"default:0" json:"threshold"`
	IsOnline        bool    `gorm:"default:1" json:"isOnline"`
	TotalDetections int     `gorm:"default:0" json:"totalDetections"`
}

type DetectionEvent struct {
	Model
	MAC         string    `gorm:"index;size:17" json:"mac"`
	Probability float32   `json:"probability"`
	Intensity   float32   `json:"intensity"`
	Timestamp   time.Time `gorm:"default:CURRENT_TIMESTAMP;index" json:"timestamp"`
}

type PairingLog struct {
	Model
	MAC        string    `gorm:"index;size:17" json:"mac"`
	Success    bool      `gorm:"default:1" json:"success"`
	Error      string    `gorm:"type:text" json:"error,omitempty"`
	RawPayload string    `gorm:"type:text" json:"rawPayload"`
	Timestamp  time.Time `gorm:"default:CURRENT_TIMESTAMP" json:"timestamp"`
}
