import axios from "axios";

const API_BASE_URL = "http://localhost:8080";

export interface Device {
  id: number;
  mac: string;
  threshold: number;
  isOnline: boolean;
  totalDetections: number;
  createdAt: string;
  updatedAt: string;
}

export interface DeviceSummary {
  mac: string;
  threshold: number;
  isOnline: boolean;
  totalDetections: number;
  detections24h: number;
}

export interface DetectionEvent {
  id: number;
  mac: string;
  probability: number;
  intensity: number;
  timestamp: string;
  createdAt: string;
  updatedAt: string;
}

export interface ConfigUpdate {
  threshold: number;
}

export interface MQTTTelemetry {
  probability: number;
  intensity: number;
  detection?: boolean;
  timestamp?: string;
}

class APIService {
  private api = axios.create({
    baseURL: API_BASE_URL,
    timeout: 5000,
  });

  async getDevices(): Promise<DeviceSummary[]> {
    const response = await this.api.get("/devices");
    return response.data;
  }

  async getDevice(mac: string): Promise<DeviceSummary> {
    const response = await this.api.get(`/devices/${mac}`);
    return response.data;
  }

  async getDetections(
    mac: string,
    limit: number = 10,
  ): Promise<DetectionEvent[]> {
    const response = await this.api.get(`/devices/${mac}/detections`, {
      params: { limit },
    });
    return response.data;
  }

  async updateConfig(mac: string, config: ConfigUpdate): Promise<void> {
    await this.api.put(`/devices/${mac}/config`, config);
  }
}

export const apiService = new APIService();
