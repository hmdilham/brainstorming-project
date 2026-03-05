# Modul Pertemuan 4 — IOT Lanjutan
# ESP-NOW Gateway Multi-Node → MQTT Bridge

---

## 1. Maksud dan Tujuan Materi

### Maksud
Membangun jaringan sensor nirkabel skala kecil: beberapa Node Sensor mengirim data via ESP-NOW ke satu Gateway ESP32, yang kemudian mem-forward data ke MQTT Broker via WiFi.

### Tujuan Pembelajaran
1. **Membangun** arsitektur Star Topology: multi-node → gateway.
2. **Mengimplementasikan** bridge ESP-NOW → MQTT di gateway.
3. **Memonitor** semua node via MQTT Explorer dan OLED gateway.

---

## 2. Teori Materi

### 2.1 Arsitektur Star Topology

```
  [Node-1: DHT22]  ──ESP-NOW──►  ┌─────────┐  ──WiFi+MQTT──►  [MQTT Broker]
  [Node-2: HC-SR04] ──ESP-NOW──► │ Gateway  │                       │
  [Node-3: LDR]     ──ESP-NOW──► │  ESP32   │               [MQTT Explorer]
                                  │ + OLED   │               [Home Assistant]
                                  └─────────┘
```

### 2.2 Broadcast vs Unicast

- **Unicast:** Kirim ke MAC Address tertentu — satu ESP32 spesifik
- **Broadcast:** Kirim ke `FF:FF:FF:FF:FF:FF` — semua ESP32 di range menerima

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Node Sensor (Kode untuk setiap node)

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <DHT.h>

DHT dht(4, DHT22);

// MAC Address Gateway — GANTI!
uint8_t gatewayMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct {
  uint8_t  nodeId;
  char     sensorType[10];
  float    value1;
  float    value2;
  uint32_t uptime;
} NodeData;

NodeData data;

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);

  esp_now_init();
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, gatewayMAC, 6);
  esp_now_add_peer(&peer);

  data.nodeId = 1;  // Ubah per node: 1, 2, 3...
  strcpy(data.sensorType, "DHT22");
}

void loop() {
  float s = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(s)) {
    data.value1 = s;
    data.value2 = h;
    data.uptime = millis() / 1000;
    esp_now_send(gatewayMAC, (uint8_t*)&data, sizeof(data));
    Serial.printf("[Node%d] T=%.1f H=%.1f\n", data.nodeId, s, h);
  }
  delay(5000);
}
```

---

### Praktikum 3.2 — Gateway: ESP-NOW → MQTT + OLED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    knolleary/PubSubClient@^2.8.0
    bblanchon/ArduinoJson@^7.0.0
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* WIFI_SSID = "NAMA_WIFI";
const char* WIFI_PASS = "PASSWORD";
const char* MQTT_HOST = "broker.hivemq.com";

typedef struct {
  uint8_t  nodeId;
  char     sensorType[10];
  float    value1;
  float    value2;
  uint32_t uptime;
} NodeData;

// Data terakhir dari setiap node (max 5)
NodeData nodes[5];
int totalNodes = 0;
int totalPackets = 0;

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len != sizeof(NodeData)) return;
  NodeData d;
  memcpy(&d, data, sizeof(d));
  totalPackets++;

  // Update atau tambah node
  int idx = -1;
  for (int i = 0; i < totalNodes; i++) {
    if (nodes[i].nodeId == d.nodeId) { idx = i; break; }
  }
  if (idx < 0 && totalNodes < 5) { idx = totalNodes++; }
  if (idx >= 0) nodes[idx] = d;

  // Forward ke MQTT
  if (mqtt.connected()) {
    JsonDocument doc;
    doc["node"] = d.nodeId;
    doc["sensor"] = d.sensorType;
    doc["v1"] = d.value1;
    doc["v2"] = d.value2;
    doc["uptime"] = d.uptime;
    String payload;
    serializeJson(doc, payload);

    char topic[40];
    snprintf(topic, sizeof(topic), "iot/node%d/data", d.nodeId);
    mqtt.publish(topic, payload.c_str());
  }

  Serial.printf("[GW] Node%d: %.1f / %.1f → MQTT\n", d.nodeId, d.value1, d.value2);
}

void reconnectMQTT() {
  if (!mqtt.connected()) {
    String id = "GW-" + String(random(0xffff), HEX);
    mqtt.connect(id.c_str());
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("Gateway  Pkts:%d", totalPackets);
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  for (int i = 0; i < totalNodes && i < 4; i++) {
    display.setCursor(0, 13 + i * 12);
    display.printf("N%d %s %.1f/%.1f",
      nodes[i].nodeId, nodes[i].sensorType,
      nodes[i].value1, nodes[i].value2);
  }

  display.setCursor(0, 56);
  display.printf("MQTT:%s", mqtt.connected() ? "OK" : "DISC");
  display.display();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mqtt.setServer(MQTT_HOST, 1883);
  reconnectMQTT();

  esp_now_init();
  esp_now_register_recv_cb(onDataRecv);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
  }
  Serial.printf("Gateway IP: %s MAC: %s\n",
    WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
}

void loop() {
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  static unsigned long t = 0;
  if (millis() - t > 500) { t = millis(); updateOLED(); }
}
```

---

## 4. Referensi

1. **Espressif.** *ESP-NOW Guide.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
2. **Random Nerd Tutorials.** *ESP-NOW Many-to-One.* [https://randomnerdtutorials.com/esp-now-many-to-one-esp32/](https://randomnerdtutorials.com/esp-now-many-to-one-esp32/)
