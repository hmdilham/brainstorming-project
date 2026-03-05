# Modul Pertemuan 16 — IOT Lanjutan
# Proyek Akhir — Presentasi & Demo Sistem

---

## 1. Maksud dan Tujuan Materi

### Maksud
Proyek akhir IOT Lanjutan mengintegrasikan seluruh elemen yang dipelajari: komunikasi data multi-protokol, Computer Vision, Home Assistant, dan kontrol aktuator. Mahasiswa mendemonstrasikan sistem IoT terpadu yang menyelesaikan masalah nyata.

---

## 2. Spesifikasi Proyek (Minimal)

| Komponen | Syarat |
|---|---|
| **Protokol Komunikasi** | Minimal 2 berbeda (ESP-NOW + MQTT, HTTP + MQTT, dll.) |
| **Sensor** | Minimal 2 jenis berbeda |
| **Computer Vision ATAU HA** | Minimal salah satu: CV (YOLO/OpenCV/MediaPipe) ATAU Home Assistant/ESPHome |
| **Remote Access** | Sistem dapat dimonitor/kontrol dari jaringan |
| **Kode** | PlatformIO/ESPHome, modular, berkomentar, terdokumentasi |
| **OLED Display** | Status lokal ditampilkan di display |

---

## 3. Contoh Ide Proyek

| Proyek | Teknologi | Kompleksitas |
|---|---|---|
| **Smart Security System** | ESP32-CAM + YOLO person detect → MQTT → buzzer + HA notifikasi Telegram | ⭐⭐⭐ |
| **Gesture Smart Home** | ESP32-CAM + MediaPipe → MQTT → relay (lampu/kipas) + HA dashboard | ⭐⭐⭐ |
| **Distributed Environment Monitor** | 3 Node ESP-NOW → Gateway → MQTT → Home Assistant dashboard + automasi | ⭐⭐⭐ |
| **Smart Attendance** | ESP32-CAM face recognition + MQTT log + HA dashboard | ⭐⭐⭐⭐ |
| **Smart Parking** | HC-SR04 multi-node + ESP-NOW gateway + MQTT + HA dashboard | ⭐⭐⭐⭐ |
| **Pan-Tilt Tracking Camera** | ESP32-CAM + OpenCV tracking → MQTT → servo pan-tilt | ⭐⭐ |

---

## 4. Format Presentasi (15 menit/kelompok)

1. **Demo alat** (5 menit): Semua fitur berjalan langsung.
2. **Arsitektur sistem** (3 menit): Diagram blok + aliran data.
3. **Highlight kode** (3 menit): Bagian paling kritis/inovatif.
4. **Tanya jawab** (4 menit).

---

## 5. Rubrik Penilaian

| Kriteria | Bobot |
|---|---|
| Fungsionalitas sistem (semua fitur berjalan) | 35% |
| Kompleksitas & integrasi teknologi | 25% |
| Kualitas kode (modular, terdokumentasi, error handling) | 20% |
| Presentasi & kemampuan teknis menjelaskan | 20% |

---

## 6. Referensi

- Semua referensi dari Pertemuan 1–15.
- **ESPHome.** [https://esphome.io/](https://esphome.io/)
- **Home Assistant.** [https://www.home-assistant.io/](https://www.home-assistant.io/)
- **PlatformIO.** [https://platformio.org/](https://platformio.org/)
- **Ultralytics YOLOv8.** [https://docs.ultralytics.com/](https://docs.ultralytics.com/)
