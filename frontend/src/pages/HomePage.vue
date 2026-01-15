<template>
  <q-page class="q-pa-md">
    <!-- Header -->
    <div class="row items-center q-mb-md">
      <div class="col">
        <h5 class="text-h5 text-weight-medium q-ma-none">
          Pest Detector Devices
        </h5>
        <p class="text-subtitle1 text-grey-7">
          Manage and monitor your pest detection devices
        </p>
      </div>
      <div class="col-auto">
        <q-badge color="green" rounded>
          {{ connectedDevicesCount }} Connected
        </q-badge>
      </div>
    </div>

    <!-- Pest Detection Alert Banner -->
    <q-banner v-if="showPestAlert" class="bg-red text-white q-mb-md" dense>
      <template v-slot:avatar>
        <q-icon name="warning" color="white" />
      </template>
      <strong>Pest Detected!</strong> {{ pestAlertMessage }}
      <template v-slot:action>
        <q-btn flat color="white" label="Dismiss" @click="dismissPestAlert" />
      </template>
    </q-banner>

    <!-- Device List -->
    <div class="row q-col-gutter-md">
      <div
        v-for="device in devices"
        :key="device.mac"
        class="col-12 col-md-6 col-lg-4"
      >
        <q-card
          class="cursor-pointer"
          @click="$router.push(`/device/${device.mac}`)"
          :class="{
            'border-green': device.isOnline,
            'border-grey': !device.isOnline,
            'border-red': device.detections24h > 0 && device.isOnline,
          }"
        >
          <q-card-section>
            <div class="row items-center no-wrap">
              <q-icon
                name="memory"
                :color="device.isOnline ? 'green' : 'grey'"
                size="24px"
                class="q-mr-sm"
              />
              <div class="text-h6">{{ device.mac }}</div>

              <q-space />

              <div class="row items-center no-wrap q-ml-sm q-gutter-x-sm">
                <q-badge
                  :color="device.isOnline ? 'green' : 'grey'"
                  :label="device.isOnline ? 'Online' : 'Offline'"
                  rounded
                />
                <q-badge v-if="device.detections24h > 0" color="red" rounded>
                  {{ device.detections24h }} Detected
                </q-badge>
              </div>
            </div>
          </q-card-section>

          <q-separator />

          <q-card-section class="q-py-sm">
            <div class="row items-center">
              <div class="col">
                <div class="text-caption text-grey-7">Threshold</div>
                <div class="text-h6">
                  {{ (device.threshold * 100).toFixed(1) }}%
                </div>
              </div>
            </div>
          </q-card-section>

          <q-card-section class="q-pt-none">
            <div class="row items-center justify-end">
              <div class="col">
                <div class="text-caption text-grey-7">Last 24 Hours</div>
                <div class="text-h6">{{ device.detections24h }}</div>
              </div>
              <div class="col">
                <div class="text-caption text-grey-7">Total</div>
                <div class="text-h6">{{ device.totalDetections || 0 }}</div>
              </div>
              <q-icon name="chevron_right" size="26px" color="grey-5" />
            </div>
          </q-card-section>
        </q-card>
      </div>
    </div>

    <!-- Loading State -->
    <div v-if="loading" class="text-center q-pa-xl">
      <q-spinner size="50px" color="primary" />
      <div class="q-mt-md">Loading devices...</div>
    </div>

    <!-- Empty State -->
    <div v-else-if="devices.length === 0" class="text-center q-pa-xl">
      <q-icon name="sensors_off" size="64px" color="grey-4" />
      <h6 class="text-h6 q-mt-md text-grey-6">No devices detected</h6>
      <p class="text-grey-6">
        Devices will appear here when they connect via MQTT
      </p>
      <q-btn
        color="primary"
        icon="refresh"
        label="Refresh"
        @click="loadDevices"
      />
    </div>

    <!-- Pest Detection Notification -->
    <q-dialog v-model="showPestNotification" persistent>
      <q-card style="width: 500px">
        <q-card-section class="row items-center">
          <q-avatar icon="warning" color="red" text-color="white" />
          <span class="q-ml-sm text-h6">Pest Detected!</span>
        </q-card-section>

        <q-card-section>
          <div class="text-body1">
            {{ pestNotification.mac }} detected pest activity with
            <strong
              >{{ (pestNotification.probability * 100).toFixed(1) }}%</strong
            >
            probability.
          </div>
          <div class="text-caption text-grey-7 q-mt-sm">
            Time: {{ formatTime(new Date()) }}
          </div>
        </q-card-section>

        <q-card-actions align="right">
          <q-btn flat label="Ignore" color="grey" v-close-popup />
          <q-btn
            label="View Details"
            color="primary"
            v-close-popup
            @click="goToDevice"
          />
        </q-card-actions>
      </q-card>
    </q-dialog>
  </q-page>
</template>

<script setup lang="ts">
import { ref, computed, onMounted, onUnmounted } from "vue";
import { useRouter } from "vue-router";
import mqtt from "mqtt";
import { format } from "date-fns";
import {
  apiService,
  type DeviceSummary,
  type MQTTTelemetry,
} from "../services/api";
import { useQuasar } from "quasar";

const router = useRouter();
const mqttClient = ref<mqtt.MqttClient | null>(null);
const devices = ref<DeviceSummary[]>([]);
const loading = ref(true);
const $q = useQuasar();

const showPestAlert = ref(false);
const pestAlertMessage = ref("");
const showPestNotification = ref(false);
const pestNotification = ref({
  mac: "",
  probability: 0,
});

let refreshInterval: ReturnType<typeof setTimeout>;

const connectedDevicesCount = computed(() => {
  return devices.value.filter((d) => d.isOnline).length;
});

const formatTime = (date: Date) => {
  return format(date, "MMM d, HH:mm");
};

const loadDevices = async () => {
  loading.value = true;
  try {
    devices.value = await apiService.getDevices();
  } catch (error) {
    console.error("Error loading devices:", error);
    $q.notify({
      type: "negative",
      message: "Failed to load devices",
      position: "top",
    });
  } finally {
    loading.value = false;
  }
};

const dismissPestAlert = () => {
  showPestAlert.value = false;
};

const goToDevice = () => {
  void router.push(`/device/${pestNotification.value.mac}`);
};

const setupMQTT = () => {
  mqttClient.value = mqtt.connect("ws://localhost:1882", {
    clean: true,
    connectTimeout: 4000,
  });

  mqttClient.value.on("connect", () => {
    console.log("Connected to MQTT broker");

    mqttClient.value?.subscribe("device/telemetry/#", { qos: 1 });

    mqttClient.value?.subscribe("device/config/#", { qos: 1 });
  });

  mqttClient.value.on("message", (topic, message) => {
    const messageStr = message.toString();
    console.log(`Received message on ${topic}:`, messageStr);

    if (topic.startsWith("device/telemetry/")) {
      const mac = topic.split("/")[2];
      try {
        const telemetry: MQTTTelemetry = JSON.parse(messageStr);
        handlePestDetection(mac!, telemetry);
      } catch (e) {
        console.error("Failed to parse telemetry:", e);
      }
    } else if (topic.startsWith("device/config/")) {
      const mac = topic.split("/")[2];
      try {
        const config = JSON.parse(messageStr);
        const deviceIndex = devices.value.findIndex((d) => d.mac === mac);
        if (deviceIndex !== -1) {
          devices.value[deviceIndex]!.threshold = config.threshold;
        }
      } catch (e) {
        console.error("Failed to parse config update:", e);
      }
    }
  });

  mqttClient.value.on("error", (error) => {
    console.error("MQTT error:", error);
  });
};

const handlePestDetection = (mac: string, telemetry: MQTTTelemetry) => {
  const deviceIndex = devices.value.findIndex((d) => d.mac === mac);
  if (deviceIndex === -1) {
    setTimeout(() => loadDevices, 500);
    return;
  }

  const device = devices.value[deviceIndex];

  const isDetection =
    telemetry.detection || telemetry.probability > device!.threshold;

  if (isDetection) {
    device!.detections24h++;
    device!.totalDetections = (device!.totalDetections || 0) + 1;

    showPestAlert.value = true;
    pestAlertMessage.value = `${mac} detected pest activity`;

    pestNotification.value = {
      mac,
      probability: telemetry.probability || 0.9,
    };
    showPestNotification.value = true;
  }

  device!.isOnline = true;
};

onMounted(async () => {
  await loadDevices();
  setupMQTT();

  refreshInterval = setInterval(() => loadDevices, 30000);
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

<style scoped>
.border-green {
  border-left: 4px solid #4caf50;
}

.border-grey {
  border-left: 4px solid #9e9e9e;
}

.border-red {
  border-left: 4px solid #f44336;
}
</style>
