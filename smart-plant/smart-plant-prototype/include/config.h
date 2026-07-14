#pragma once

// ─────────────────────────────────────────────────────────────
//  HARDWARE PINS  (NodeMCU v3 / ESP8266)
// ─────────────────────────────────────────────────────────────
#define PIN_RELAY     14    // D5 — Relay 1-ch, active LOW
#define PIN_SOIL_ADC  A0    // Capacitive soil moisture sensor
#define PIN_DHT22     12    // D6 — DHT22 DATA (1-wire, pull-up 10kΩ ke 3V3)
// OLED I2C uses Wire default: SDA = D2 (GPIO4), SCL = D1 (GPIO5)

// ─────────────────────────────────────────────────────────────
//  OLED SSD1306 (0.96" I2C 128×64)
// ─────────────────────────────────────────────────────────────
#define OLED_WIDTH    128
#define OLED_HEIGHT    64
#define OLED_RESET     -1   // shared reset dengan MCU
#define OLED_ADDR    0x3C

// ─────────────────────────────────────────────────────────────
//  SOIL SENSOR CALIBRATION  (ganti setelah kalibrasi nyata)
// ─────────────────────────────────────────────────────────────
#define SOIL_RAW_DRY  850   // ADC value di udara kering
#define SOIL_RAW_WET  380   // ADC value di air penuh
#define SAMPLE_COUNT    8   // moving average window
#define SOIL_VALID_MARGIN 150 // toleransi raw di luar [wet, dry] sebelum dianggap sensor lepas

// ─────────────────────────────────────────────────────────────
//  DHT22 — Suhu & Kelembapan Udara
// ─────────────────────────────────────────────────────────────
#define DHT_READ_INTERVAL  2500UL   // minimum 2 detik antar pembacaan DHT22

// ─────────────────────────────────────────────────────────────
//  PUMP CONTROL — Hysteresis + State Machine
// ─────────────────────────────────────────────────────────────
#define SOIL_DRY_THRESHOLD   30    // % → START pump (tanah kering)
#define SOIL_WET_THRESHOLD   55    // % → STOP  pump (tanah cukup basah)
#define PUMP_MIN_ON_MS     8000UL  // minimal pompa menyala 8 detik
#define PUMP_MAX_ON_MS    30000UL  // safety cutoff 30 detik
#define COOLDOWN_MS       90000UL  // jeda 90 detik sebelum cek lagi

// ─────────────────────────────────────────────────────────────
//  TIMING (ms)
// ─────────────────────────────────────────────────────────────
#define SENSOR_INTERVAL    3000UL   // baca sensor setiap 3 detik
#define OLED_INTERVAL      2000UL   // refresh OLED setiap 2 detik
#define MQTT_PUB_INTERVAL 10000UL   // publish MQTT setiap 10 detik

// ─────────────────────────────────────────────────────────────
//  MQTT TOPICS
// ─────────────────────────────────────────────────────────────
#define MQTT_CLIENT_ID         "SmartPlantProto"
#define MQTT_TOPIC_SOIL        "smartplant/proto/soil"
#define MQTT_TOPIC_TEMP        "smartplant/proto/temp"
#define MQTT_TOPIC_HUMIDITY    "smartplant/proto/humidity"
#define MQTT_TOPIC_PUMP_STATE  "smartplant/proto/pump/state"
#define MQTT_TOPIC_PUMP_SET    "smartplant/proto/pump/set"
#define MQTT_TOPIC_STATUS      "smartplant/proto/status"
#define MQTT_TOPIC_AVAIL       "smartplant/proto/availability"

// Home Assistant auto-discovery
#define HA_DISC_SOIL      "homeassistant/sensor/smartplant_proto_soil/config"
#define HA_DISC_TEMP      "homeassistant/sensor/smartplant_proto_temp/config"
#define HA_DISC_HUMIDITY  "homeassistant/sensor/smartplant_proto_humidity/config"
#define HA_DISC_PUMP      "homeassistant/switch/smartplant_proto_pump/config"

// ─────────────────────────────────────────────────────────────
//  DEVICE INFO
// ─────────────────────────────────────────────────────────────
#define DEVICE_NAME      "Smart Plant Prototype"
#define FIRMWARE_VER     "1.0.0"
#define AP_SSID          "SmartPlant-Proto"
#define AP_PASSWORD      "smartplant"
