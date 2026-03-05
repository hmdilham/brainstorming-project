# Modul Pertemuan 15 — IOT Lanjutan
# Arsitektur Sistem Terpadu & Persiapan Proyek Akhir

---

## 1. Maksud dan Tujuan Materi

### Maksud
Menggabungkan seluruh elemen yang dipelajari: komunikasi antar-ESP32, ESP-NOW, HTTP, MQTT, Home Assistant, ESPHome, dan Computer Vision menjadi satu arsitektur sistem terpadu. Mahasiswa mempersiapkan desain proyek akhir.

### Tujuan Pembelajaran
1. **Mendesain** arsitektur sistem IoT terpadu multi-layer.
2. **Memilih** protokol yang tepat untuk setiap komponen.
3. **Menerapkan** best practices: modularisasi, error handling, deep sleep.
4. **Memvalidasi** desain proyek akhir.

---

## 2. Teori Materi

### 2.1 Arsitektur Sistem Terpadu

```
  ┌─ Sensor Layer ─────────────────────────────────────┐
  │  [Node 1: DHT22]  [Node 2: HC-SR04]  [Node 3: LDR]│
  │       │                  │                  │      │
  └───────┼──────────────────┼──────────────────┼──────┘
          │  ESP-NOW         │  ESP-NOW         │
  ┌───────▼──────────────────▼──────────────────▼──────┐
  │  Gateway ESP32                                      │
  │  ESP-NOW Receiver → MQTT Publisher                  │
  │  + OLED Status Display                              │
  └──────────────────────┬─────────────────────────────┘
                         │  WiFi / MQTT
  ┌──────────────────────▼─────────────────────────────┐
  │  MQTT Broker (Mosquitto)                            │
  └──────────┬────────────┬────────────┬───────────────┘
             │            │            │
  ┌──────────▼──┐  ┌──────▼─────┐  ┌──▼────────────────┐
  │ Home        │  │ Python CV  │  │ ESP32 Subscriber   │
  │ Assistant   │  │ Server     │  │ (Aktuator)         │
  │ - Dashboard │  │ - YOLO     │  │ - Servo, Relay     │
  │ - Automasi  │  │ - MediaPipe│  │ - Buzzer, LED      │
  │ - Telegram  │  │ - OpenCV   │  │ - OLED Status      │
  └─────────────┘  └────────────┘  └────────────────────┘
```

### 2.2 Pemilihan Protokol

| Kebutuhan | Protokol Terbaik |
|---|---|
| Sensor → Gateway (tanpa router) | **ESP-NOW** |
| Gateway → Cloud/Broker | **MQTT** (via WiFi) |
| ESP32 → REST API internet | **HTTP GET/POST** |
| Smart home dashboard | **ESPHome → Home Assistant** |
| CV processing | **Python + OpenCV/YOLO** |
| Notifikasi user | **HA → Telegram** |

### 2.3 Deep Sleep untuk Battery-Powered Node

```cpp
// Node sensor bertenaga baterai:
// Bangun → baca sensor → kirim ESP-NOW → tidur 5 menit

#include <esp_sleep.h>

#define uS_TO_S_FACTOR 1000000ULL
#define SLEEP_SECONDS  300  // 5 menit

void setup() {
  // Baca sensor...
  // Kirim ESP-NOW...
  
  esp_sleep_enable_timer_wakeup(SLEEP_SECONDS * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
  // Program berhenti di sini. Saat bangun, mulai dari setup() lagi.
}
```

---

## 3. Workshop

1. Setiap kelompok menggambar arsitektur sistem proyek mereka.
2. Validasi pilihan komponen, protokol, dan tools.
3. Debugging dan penyelesaian masalah teknis.
4. Konsultasi desain dengan dosen.

---

## 4. Referensi

- Semua referensi dari Pertemuan 1–14.
