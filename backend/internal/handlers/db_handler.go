package handlers

import (
	"time"

	"gorm.io/gorm"

	"pest-detector/internal/models"
)

type DBHandler struct {
	db *gorm.DB
}

func NewDBHandler(db *gorm.DB) *DBHandler {
	return &DBHandler{db: db}
}

func (h *DBHandler) GetDevices() ([]models.DeviceSummary, error) {
	var statuses []models.Device
	err := h.db.Find(&statuses).Error
	if err != nil {
		return nil, err
	}

	var summaries []models.DeviceSummary
	for _, status := range statuses {
		summary, err := h.GetDevice(status.MAC)
		if err != nil {
			return nil, err
		}
		summaries = append(summaries, *summary)
	}

	return summaries, nil
}

func (h *DBHandler) GetDevice(mac string) (*models.DeviceSummary, error) {
	var device models.Device
	if err := h.db.Where("mac = ?", mac).First(&device).Error; err != nil {
		return nil, err
	}

	detections24h, err := h.getDetectionsCount24h(mac)
	if err != nil {
		detections24h = 0
	}

	return &models.DeviceSummary{
		Device:        device,
		Detections24h: int(detections24h),
	}, nil
}

func (h *DBHandler) GetDetections(mac string) ([]models.DetectionEvent, error) {
	var detections []models.DetectionEvent
	err := h.db.Where("mac = ?", mac).
		Order("timestamp DESC").
		Find(&detections).Error
	return detections, err
}

func (h *DBHandler) UpdateConfig(mac string, threshold float32) error {
	var device models.Device
	if err := h.db.Where("mac = ?", mac).First(&device).Error; err != nil {
		return err
	}

	device.Threshold = threshold

	return h.db.Save(&device).Error
}

func (h *DBHandler) UpdateDevice(mac string, isOnline bool, detection bool) error {
	var device models.Device
	if err := h.db.Where("mac = ?", mac).First(&device).Error; err != nil {
		return err
	}

	device.IsOnline = isOnline
	if detection {
		device.TotalDetections++
	}

	return h.db.Save(&device).Error
}

func (h *DBHandler) AddDetectionEvent(mac string, probability float32, intensity float32) error {
	detection := models.DetectionEvent{
		MAC:         mac,
		Probability: probability,
		Intensity:   intensity,
	}

	return h.db.Create(&detection).Error
}

func (h *DBHandler) CleanupOldData(days int) error {
	cutoffTime := time.Now().AddDate(0, 0, -days)

	if err := h.db.Where("timestamp < ?", cutoffTime).
		Delete(&models.DetectionEvent{}).Error; err != nil {
		return err
	}

	return nil
}

func (h *DBHandler) getDetectionsCount24h(mac string) (int64, error) {
	twentyFourHoursAgo := time.Now().Add(-24 * time.Hour)

	var count int64
	err := h.db.Model(&models.DetectionEvent{}).
		Where("mac = ? AND timestamp >= ?", mac, twentyFourHoursAgo).
		Count(&count).Error

	return count, err
}

func (h *DBHandler) createOrUpdateDevice(mac string, threshold float32) (*models.Device, error) {
	device := models.Device{
		MAC:       mac,
		Threshold: threshold,
		IsOnline:  true,
	}
	var existing models.Device
	result := h.db.Where("mac = ?", mac).First(&existing)

	if result.Error == gorm.ErrRecordNotFound {
		return nil, h.db.Create(&device).Error
	} else if result.Error != nil {
		return nil, result.Error
	}

	device.ID = existing.ID
	device.CreatedAt = existing.CreatedAt
	device.Threshold = existing.Threshold
	return &device, h.db.Save(&device).Error
}

func (h *DBHandler) logPairing(mac string, payload string, success bool, errMsg string) error {
	pairingLog := models.PairingLog{
		MAC:        mac,
		Success:    success,
		Error:      errMsg,
		RawPayload: payload,
	}

	return h.db.Create(&pairingLog).Error
}
