# Modul Pertemuan 3 — IOT Lanjutan
# ESP-NOW — Komunikasi Nirkabel Point-to-Point

---

## 1. Maksud dan Tujuan Materi

### Maksud
ESP-NOW adalah protokol proprietary Espressif untuk komunikasi nirkabel **langsung** antar ESP32 tanpa router WiFi. Dengan latensi <1ms dan jangkauan ~200m (line of sight), ESP-NOW ideal untuk jaringan sensor lokal, remote control, dan sistem real-time.

### Tujuan Pembelajaran
1. **Menjelaskan** konsep ESP-NOW: MAC Address, peer, callback.
2. **Mengirim** struct data terstruktur via ESP-NOW.
3. **Menerima** dan menampilkan data pada OLED di sisi receiver.
4. **Menguji** jangkauan komunikasi ESP-NOW.

---

## 2. Teori Materi

### 2.1 ESP-NOW vs WiFi vs BLE

| Aspek | WiFi | BLE | **ESP-NOW** |
|---|---|---|---|
| Butuh Router | ✅ Ya | ❌ Tidak | **❌ Tidak** |
| Latensi | ~10-100 ms | ~7.5 ms | **<1 ms** |
| Range | ~50 m (indoor) | ~10-30 m | **~200 m (LOS)** |
| Max peers | N/A | 7 | **20 (encrypted), 6 (ESP32)** |
| Payload | Variable | ~20 bytes | **250 bytes** |
| Daya | Tinggi (~240 mA) | Rendah (~15 mA) | **Sangat rendah** |
| Ide penggunaan | Internet, cloud | Smartphone, wearable | **Sensor network, remote** |

### 2.2 Cara Kerja ESP-NOW

```
  ESP32-A (Sender)                    ESP32-B (Receiver)
  ┌──────────────┐                    ┌──────────────┐
  │  MAC: AA:BB.. │ ───ESP-NOW────►   │  MAC: CC:DD.. │
  │              │     (250 byte      │              │
  │  OnDataSent  │      max)          │  OnDataRecv  │
  │  callback    │                    │  callback    │
  └──────────────┘                    └──────────────┘

  1. Sender mendaftarkan MAC Address Receiver sebagai "peer"
  2. Sender mengirim data (struct, max 250 bytes)
  3. Callback OnDataSent dipanggil → status: berhasil/gagal
  4. Receiver callback OnDataRecv dipanggil → data diterima
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Sender: DHT22 → ESP-NOW

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/DHT sensor library@^1.4.6
    adafruit/Adafruit Unified Sensor@^1.1.14
```

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <DHT.h>

DHT dht(4, DHT22);

// === GANTI dengan MAC Address ESP32 Receiver! ===
uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Ganti!

// Struct data (harus sama di sender & receiver)
typedef struct {
  uint8_t  nodeId;
  float    suhu;
  float    hum;
  uint32_t timestamp;
} SensorData;

SensorData txData;
int sendCount = 0;
bool lastSendOK = false;

void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  lastSendOK = (status == ESP_NOW_SEND_SUCCESS);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);

  Serial.printf("Sender MAC: %s\n", WiFi.macAddress().c_str());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init gagal!"); while(1) delay(1);
  }
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  txData.nodeId = 1;
}

void loop() {
  float s = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(s)) {
    txData.suhu = s;
    txData.hum = h;
    txData.timestamp = millis();
    sendCount++;

    esp_now_send(peerMAC, (uint8_t*)&txData, sizeof(txData));
    Serial.printf("[#%d] TX: T=%.1f H=%.1f %s\n",
                  sendCount, s, h, lastSendOK ? "OK" : "FAIL");
  }

  delay(3000);
}
```

---

### Praktikum 3.2 — Receiver: OLED Display

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

typedef struct {
  uint8_t  nodeId;
  float    suhu;
  float    hum;
  uint32_t timestamp;
} SensorData;

SensorData rxData;
int rxCount = 0;
unsigned long lastRxTime = 0;

void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(SensorData)) {
    memcpy(&rxData, data, sizeof(rxData));
    rxCount++;
    lastRxTime = millis();
    Serial.printf("[#%d] RX Node%d: T=%.1f H=%.1f\n",
                  rxCount, rxData.nodeId, rxData.suhu, rxData.hum);
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ESP-NOW Receiver");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("Node : %d", rxData.nodeId);
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.printf("%.1fC", rxData.suhu);
  display.setCursor(75, 25);
  display.printf("%.0f%%", rxData.hum);

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.printf("Paket: %d", rxCount);
  display.setCursor(0, 55);
  unsigned long ago = (millis() - lastRxTime) / 1000;
  display.printf("Last : %lus ago", lastRxTime > 0 ? ago : 0);

  display.display();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.printf("Receiver MAC: %s\n", WiFi.macAddress().c_str());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init gagal!"); while(1) delay(1);
  }
  esp_now_register_recv_cb(onDataRecv);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }
}

void loop() {
  static unsigned long lastOLED = 0;
  if (millis() - lastOLED > 500) {
    lastOLED = millis();
    updateOLED();
  }
}
```

---

## 4. Referensi

1. **Espressif.** *ESP-NOW API.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
2. **Random Nerd Tutorials.** *ESP-NOW.* [https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/](https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/)
