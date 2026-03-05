# Walkthrough: Kurikulum IoT Dasar & Lanjutan (16 Pertemuan)

Kedua file kurikulum — **16 pertemuan** masing-masing, proyek akhir di P16.

---

## Perbedaan Kunci Dasar vs Lanjutan

| Aspek | IoT Dasar (P5–7) | IoT Lanjutan (mulai P1) |
|---|---|---|
| **Protokol** | UART, I2C, SPI | UART, I2C, ESP-NOW, HTTP, MQTT |
| **Konteks** | ESP32 ↔ **sensor/aktuator** | ESP32 ↔ **ESP32 lain** |
| **Contoh** | ESP32 → DHT22, OLED, SD Card | ESP32-A → ESP32-B (bagi tugas) |

---

## Struktur dasar.md — 16 Pertemuan

| # | Topik | Highlight |
|---|---|---|
| 1 | Pengenalan IoT & ESP32 | Ekosistem IoT, varian board |
| 2 | Instalasi IDE & First Project | Arduino IDE 2.x + PlatformIO |
| 3 | GPIO Output — Digital & PWM | LED Dimming, LED RGB |
| 4 | GPIO Input — Digital & Analog | Debouncing, ADC 12-bit |
| **5** | **UART (ESP32 ↔ Sensor/Modul)** | Komunikasi 2 arah + catatan konteks |
| **6** | **I2C (ESP32 ↔ Sensor/Display)** | DHT22 + OLED, multi-device scan |
| **7** | **SPI (ESP32 ↔ Modul Penyimpanan)** | SD Card data logger |
| 8 | Sensor | PIR, LDR, HC-SR04, BMP280 |
| 9 | Aktuator | Servo, Relay, Motor DC + L298N |
| 10 | Komunikasi WiFi | STA/AP mode, NTP real-time clock |
| 11 | Bluetooth BLE | BLE Server/Client, nRF Connect |
| 12 | Library Populer | WiFiManager, ArduinoJson, Preferences |
| 13 | Web Server ESP32 | ESPAsyncWebServer, REST API JSON |
| 14 | Platform IoT (Blynk & MQTT dasar) | Blynk 2.0, PubSubClient preview |
| 15 | Integrasi & Persiapan Proyek | Non-blocking `millis()`, konsultasi |
| **16** | **Proyek Akhir — Presentasi** | Demo + rubrik penilaian |

---

## Struktur lanjutan.md — 16 Pertemuan (DIREVISI)

| # | Bagian | Topik |
|---|---|---|
| **1** | **Inter-ESP32** | **UART ESP32↔ESP32** — protokol sama, konteks beda |
| **2** | **Inter-ESP32** | **I2C Master/Slave ESP32↔ESP32** |
| 3 | ESP-NOW | Point-to-Point (struct data, callback) |
| 4 | ESP-NOW | Gateway Multi-Node → Bridge ke MQTT |
| 5 | REST API | HTTP GET — Konsumsi API publik |
| 6 | REST API | HTTP POST — Kirim data ke server |
| 7 | MQTT | Konsep Pub/Sub + Setup Broker Mosquitto |
| 8 | MQTT | Implementasi ESP32 Publisher & Subscriber |
| 9 | Edge AI | TensorFlow Lite Micro — konsep + deploy |
| 10 | Edge AI | Edge Impulse — Gesture Classification |
| 11 | Computer Vision | ESP32-CAM Setup & MJPEG Streaming |
| 12 | Computer Vision | Face Detection & Recognition (ESP-WHO) |
| 13 | Computer Vision | OpenCV Color Tracking + MQTT feedback |
| 14 | Computer Vision | YOLO + MediaPipe Gesture → MQTT |
| 15 | Persiapan | Arsitektur sistem terpadu + konsultasi |
| **16** | **Proyek Akhir** | Demo + rubrik penilaian |

---

## File yang Diperbarui

- [dasar.md](file:///mnt/DataShare/MyHome/Documents/Downloads/claude-workspace/project-brainstrom/silabus-iot-lanjutan-2026/dasar.md)
- [lanjutan.md](file:///mnt/DataShare/MyHome/Documents/Downloads/claude-workspace/project-brainstrom/silabus-iot-lanjutan-2026/lanjutan.md)
