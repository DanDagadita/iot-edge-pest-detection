<template>
  <q-page class="q-pa-md">
    <!-- Back button and header -->
    <div class="row items-center q-mb-lg">
      <div class="col-auto">
        <q-btn
          flat
          round
          icon="arrow_back"
          @click="$router.push('/')"
          class="q-mr-md"
        />
      </div>
      <div class="col">
        <h6 class="text-h6 q-ma-none">{{ deviceMac }}</h6>
      </div>
      <div class="col-auto">
        <q-badge
          :color="device.isOnline ? 'green' : 'grey'"
          :label="device.isOnline ? 'Online' : 'Offline'"
          rounded
          size="lg"
        />
      </div>
    </div>

    <!-- Recent Detection Alert -->
    <q-banner
      v-if="recentDetection"
      class="bg-red text-white q-mb-md"
      rounded
      dense
    >
      <template v-slot:avatar>
        <q-icon name="warning" color="white" />
      </template>
      <strong>Recent Pest Detection!</strong>
      Probability: {{ (recentDetection.probability * 100).toFixed(1) }}% at
      {{ formatTime(recentDetection.timestamp) }}
    </q-banner>

    <div class="row q-col-gutter-lg">
      <!-- Left Column: Device Info -->
      <div class="col-12 col-md-8">
        <!-- Status Card -->
        <q-card class="q-mb-lg">
          <q-card-section>
            <div class="row items-center">
              <div class="col">
                <div class="text-h6">Device Information</div>
                <div class="text-subtitle1 q-mt-sm">
                  <q-icon
                    :name="device.isOnline ? 'wifi' : 'wifi_off'"
                    :color="device.isOnline ? 'green' : 'grey'"
                    class="q-mr-sm"
                  />
                  {{
                    device.isOnline
                      ? "Connected and monitoring"
                      : "Disconnected"
                  }}
                </div>
              </div>
              <div class="col-auto">
                <q-icon
                  name="memory"
                  size="64px"
                  :color="device.isOnline ? 'primary' : 'grey-5'"
                />
              </div>
            </div>
          </q-card-section>

          <q-separator />

          <q-card-section>
            <div class="row">
              <div class="col-6 q-pa-sm">
                <div class="text-caption text-grey-7">MAC Address</div>
                <div class="text-body1">{{ device.mac }}</div>
              </div>
              <div class="col-6 q-pa-sm">
                <div class="text-caption text-grey-7">Detection Threshold</div>
                <div class="text-body1">
                  {{ (device.threshold * 100).toFixed(1) }}%
                </div>
              </div>
              <div class="col-6 q-pa-sm">
                <div class="text-caption text-grey-7">Last 24 Hours</div>
                <div class="text-body1">
                  {{ device.detections24h }} detections
                </div>
              </div>
              <div class="col-6 q-pa-sm">
                <div class="text-caption text-grey-7">Total Detections</div>
                <div class="text-body1">{{ device.totalDetections }}</div>
              </div>
            </div>
          </q-card-section>
        </q-card>

        <!-- Detection History -->
        <q-card>
          <q-card-section>
            <div class="row items-center justify-between">
              <div class="text-h6">Detection History</div>
              <q-btn
                icon="refresh"
                flat
                round
                @click="loadDetections"
                :loading="loadingDetections"
              />
            </div>
          </q-card-section>

          <q-card-section class="q-mx-md q-pt-none">
            <q-timeline color="secondary">
              <q-timeline-entry
                v-for="(event, index) in detectionHistory"
                :key="event.id"
                :title="`Pest detected (${(event.probability * 100).toFixed(1)}%)`"
                :subtitle="formatTime(event.timestamp)"
                :icon="getSeverityIcon(event.probability)"
                :color="getSeverityColor(event.probability)"
              >
                <div>Intensity: {{ (event.intensity * 100).toFixed(1) }}%</div>
                <div v-if="index === 0" class="text-caption text-green">
                  <q-icon name="circle" size="8px" /> Latest
                </div>
              </q-timeline-entry>
            </q-timeline>

            <div
              v-if="detectionHistory.length === 0"
              class="text-center q-py-lg"
            >
              <q-icon name="notifications_off" size="48px" color="grey-4" />
              <div class="text-grey-6 q-mt-sm">No detections recorded yet</div>
            </div>
          </q-card-section>
        </q-card>
      </div>

      <!-- Right Column: Controls -->
      <div class="col-12 col-md-4">
        <!-- Quick Actions -->
        <q-card class="q-mb-lg">
          <q-card-section>
            <div class="text-h6">Quick Actions</div>
          </q-card-section>

          <q-card-section class="q-gutter-md q-pt-none">
            <q-btn
              color="primary"
              icon="refresh"
              label="Refresh Device"
              @click="refreshDeviceData"
              :loading="refreshing"
              block
            />
            <q-btn
              color="secondary"
              icon="notifications"
              label="Test Alert"
              @click="testAlert"
              block
            />
          </q-card-section>
        </q-card>

        <!-- Settings -->
        <q-card>
          <q-card-section>
            <div class="text-h6">Device Settings</div>
          </q-card-section>

          <q-card-section class="q-gutter-y-md q-pt-none">
            <div>
              <div class="text-caption q-mb-xs">Detection Sensitivity</div>
              <div class="text-body2 q-mb-xs">
                Current threshold: {{ (device.threshold * 100).toFixed(1) }}%
              </div>
              <q-slider
                v-model="threshold"
                :min="50"
                :max="99"
                :step="1"
                markers
                label
                label-always
                :label-value="`${threshold}%`"
              />
              <div class="row justify-between q-mt-xs">
                <div class="text-caption text-grey-7">Less sensitive</div>
                <div class="text-caption text-grey-7">More sensitive</div>
              </div>
            </div>

            <q-btn
              color="primary"
              label="Update Settings"
              @click="updateThreshold"
              :loading="updatingThreshold"
              block
            />
          </q-card-section>
        </q-card>

        <!-- Real-time Telemetry -->
        <q-card v-if="device.isOnline" class="q-mt-md">
          <q-card-section>
            <div class="text-h6">Live Telemetry</div>
          </q-card-section>

          <q-card-section>
            <div v-if="latestTelemetry" class="q-gutter-y-md">
              <div>
                <div class="text-caption text-grey-7">Current Probability</div>
                <div class="row items-center">
                  <q-linear-progress
                    :value="latestTelemetry.probability"
                    :color="getProbabilityColor(latestTelemetry.probability)"
                    style="height: 20px; flex-grow: 1"
                    class="q-mr-sm"
                  />
                  <span class="text-h6"
                    >{{ (latestTelemetry.probability * 100).toFixed(1) }}%</span
                  >
                </div>
                <div
                  v-if="latestTelemetry.probability > device.threshold"
                  class="text-red text-caption q-mt-xs"
                >
                  <q-icon name="warning" size="14px" /> Above threshold -
                  Potential pest
                </div>
              </div>

              <div>
                <div class="text-caption text-grey-7">Intensity Level</div>
                <q-linear-progress
                  :value="latestTelemetry.intensity"
                  color="orange"
                  style="height: 20px"
                />
                <div class="text-caption text-center">
                  {{ (latestTelemetry.intensity * 100).toFixed(1) }}%
                </div>
              </div>

              <div class="text-caption text-grey-7 text-center">
                Updated {{ formatRelativeTime(latestTelemetry.timestamp) }}
              </div>
            </div>

            <div v-else class="text-center q-py-md">
              <q-icon name="query_stats" size="48px" color="grey-4" />
              <div class="text-grey-6 q-mt-sm">
                Waiting for telemetry data...
              </div>
            </div>
          </q-card-section>
        </q-card>
      </div>
    </div>
  </q-page>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from "vue";
import { useRoute } from "vue-router";
import mqtt from "mqtt";
import { format, formatDistanceToNow } from "date-fns";
import {
  apiService,
  type DeviceSummary,
  type DetectionEvent,
  type MQTTTelemetry,
} from "../services/api";
import { useQuasar } from "quasar";

const route = useRoute();
const deviceMac = route.params.mac as string;
const $q = useQuasar();

const device = ref<DeviceSummary>({
  mac: deviceMac,
  threshold: 0.9,
  isOnline: false,
  totalDetections: 0,
  detections24h: 0,
});

const detectionHistory = ref<DetectionEvent[]>([]);
const loadingDetections = ref(false);
const recentDetection = ref<DetectionEvent | null>(null);

const threshold = ref(90);
const updatingThreshold = ref(false);

const mqttClient = ref<mqtt.MqttClient | null>(null);
const latestTelemetry = ref<MQTTTelemetry | null>(null);

const refreshing = ref(false);
let refreshInterval: ReturnType<typeof setTimeout>;

const formatTime = (timestamp: string) => {
  if (!timestamp) return "Never";
  return format(new Date(timestamp), "MMM d, HH:mm:ss");
};

const formatRelativeTime = (timestamp?: string) => {
  if (!timestamp) return "just now";
  return formatDistanceToNow(new Date(timestamp), { addSuffix: true });
};

const getSeverityColor = (probability: number): string => {
  if (probability > 0.9) return "red";
  if (probability > 0.7) return "orange";
  return "green";
};

const getSeverityIcon = (probability: number): string => {
  if (probability > 0.9) return "warning";
  if (probability > 0.7) return "notifications";
  return "info";
};

const getProbabilityColor = (probability: number): string => {
  if (probability > 0.9) return "red";
  if (probability > 0.7) return "orange";
  if (probability > 0.5) return "yellow";
  return "green";
};

const loadDeviceData = async () => {
  try {
    device.value = await apiService.getDevice(deviceMac);
    threshold.value = device.value.threshold * 100;
  } catch (error) {
    console.error("Error loading device:", error);
    $q.notify({
      type: "negative",
      message: "Failed to load device data",
      position: "top",
    });
  }
};

const loadDetections = async () => {
  loadingDetections.value = true;
  try {
    const detections = await apiService.getDetections(deviceMac, 10);
    detectionHistory.value = detections;
    recentDetection.value = detections.length > 0 ? detections[0]! : null;
  } catch (error) {
    console.error("Error loading detections:", error);
    $q.notify({
      type: "negative",
      message: "Failed to load detections",
      position: "top",
    });
  } finally {
    loadingDetections.value = false;
  }
};

const updateThreshold = async () => {
  updatingThreshold.value = true;
  try {
    await apiService.updateConfig(deviceMac, {
      threshold: threshold.value / 100,
    });

    device.value.threshold = threshold.value / 100;

    if (mqttClient.value?.connected) {
      mqttClient.value.publish(
        `device/config/${deviceMac}`,
        JSON.stringify({
          threshold: threshold.value / 100,
        }),
      );
    }

    $q.notify({
      type: "positive",
      message: "Threshold updated successfully",
      position: "top",
    });
  } catch (error) {
    console.error("Error updating threshold:", error);
    $q.notify({
      type: "negative",
      message: "Failed to update threshold",
      position: "top",
    });
  } finally {
    updatingThreshold.value = false;
  }
};

const refreshDeviceData = async () => {
  refreshing.value = true;
  await Promise.all([loadDeviceData(), loadDetections()]);
  refreshing.value = false;
  $q.notify({
    type: "positive",
    message: "Device data refreshed",
    position: "top",
  });
};

const testAlert = () => {
  const testTelemetry: MQTTTelemetry = {
    probability: 0.95,
    intensity: 0.8,
    detection: true,
    timestamp: new Date().toISOString(),
  };

  handleTelemetryUpdate(testTelemetry);

  $q.notify({
    type: "info",
    message: "Test alert triggered",
    position: "top",
  });
};

const setupMQTT = () => {
  mqttClient.value = mqtt.connect("ws://localhost:1882", {
    clean: true,
    connectTimeout: 4000,
  });

  mqttClient.value.on("connect", () => {
    console.log("Connected to MQTT broker for device", deviceMac);

    mqttClient.value?.subscribe(`device/telemetry/${deviceMac}`, { qos: 1 });

    mqttClient.value?.subscribe(`device/config/${deviceMac}`, { qos: 1 });
  });

  mqttClient.value.on("message", (topic, message) => {
    const messageStr = message.toString();
    console.log(`Received message on ${topic}:`, messageStr);

    if (topic === `device/telemetry/${deviceMac}`) {
      try {
        const telemetry: MQTTTelemetry = JSON.parse(messageStr);
        handleTelemetryUpdate(telemetry);
      } catch (e) {
        console.error("Failed to parse telemetry:", e);
      }
    } else if (topic === `device/config/${deviceMac}`) {
      try {
        const config = JSON.parse(messageStr);
        device.value.threshold = config.threshold;
        threshold.value = config.threshold * 100;
      } catch (e) {
        console.error("Failed to parse config update:", e);
      }
    }
  });

  mqttClient.value.on("error", (error) => {
    console.error("MQTT error:", error);
  });
};

const handleTelemetryUpdate = (telemetry: MQTTTelemetry) => {
  latestTelemetry.value = telemetry;

  device.value.isOnline = true;

  const isDetection =
    telemetry.detection || telemetry.probability > device.value.threshold;

  if (isDetection) {
    const newDetection: DetectionEvent = {
      id: Date.now(),
      mac: deviceMac,
      probability: telemetry.probability,
      intensity: telemetry.intensity,
      timestamp: telemetry.timestamp || new Date().toISOString(),
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString(),
    };

    detectionHistory.value.unshift(newDetection);
    recentDetection.value = newDetection;

    device.value.detections24h++;
    device.value.totalDetections++;
  }
};

onMounted(async () => {
  await loadDeviceData();
  await loadDetections();
  setupMQTT();
  refreshInterval = setInterval(() => refreshDeviceData, 30000);
});

onUnmounted(() => {
  if (refreshInterval) {
    clearInterval(refreshInterval);
  }
  if (mqttClient.value) {
    mqttClient.value.end();
  }
});
</script>
