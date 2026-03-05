# Modul Pertemuan 16 — IOT Dasar
# Proyek Akhir — Presentasi & Demo

---

## 1. Maksud dan Tujuan Materi

### Maksud
Proyek akhir adalah kulminasi dari seluruh pembelajaran di IOT Dasar. Mahasiswa mengintegrasikan sensor, aktuator, komunikasi, dan platform IoT menjadi satu sistem fungsional yang menyelesaikan masalah nyata.

### Tujuan Pembelajaran
1. **Merancang** dan **membangun** sistem IoT fungsional secara mandiri.
2. **Mengintegrasikan** minimal 2 sensor, 1 aktuator, konektivitas WiFi, dan dashboard.
3. **Mempresentasikan** proyek: demo langsung, arsitektur, dan kode utama.

---

## 2. Spesifikasi Proyek (Minimal)

| Komponen | Syarat Minimal |
|---|---|
| **Sensor** | Minimal 2 jenis berbeda |
| **Aktuator** | Minimal 1 jenis |
| **Konektivitas** | Terhubung WiFi, akses remote |
| **Dashboard** | Web Server / Blynk / Home Assistant |
| **Kode** | PlatformIO atau ESPHome, modular, berkomentar |
| **OLED** | Menampilkan status/data di display lokal |

---

## 3. Contoh Ide Proyek

| Proyek | Sensor | Aktuator | Platform |
|---|---|---|---|
| Penyiram Tanaman Otomatis | Soil moisture + DHT22 | Pompa (relay) | Home Assistant + ESPHome |
| Monitoring Kualitas Udara | MQ-135 + DHT22 + BMP280 | Buzzer + LED | Web Server + grafik |
| Smart Security Door | PIR + HC-SR04 | Servo lock + Buzzer | Blynk notifikasi |
| Dashboard Cuaca Lokal | DHT22 + BMP280 + LDR | LED RGB + OLED | Web Server + NTP |
| Smart Fish Tank | DS18B20 + LDR | Heater relay + lampu | Home Assistant |

---

## 4. Format Presentasi (14 menit/kelompok)

1. **Demo alat** (5 menit): Semua fitur bekerja secara langsung.
2. **Arsitektur & skematik** (3 menit): Diagram blok + skema rangkaian.
3. **Highlight kode** (3 menit): Bagian kode paling kritis.
4. **Tanya jawab** (3 menit).

---

## 5. Rubrik Penilaian

| Kriteria | Bobot |
|---|---|
| Fungsionalitas (semua fitur berjalan) | 40% |
| Kualitas kode (modular, bersih, berkomentar) | 20% |
| Kreativitas & kompleksitas | 20% |
| Presentasi & kemampuan menjelaskan | 20% |

---

## 6. Referensi

- Semua referensi dari Pertemuan 1–15.
- **ESPHome.** [https://esphome.io/](https://esphome.io/)
- **Home Assistant.** [https://www.home-assistant.io/](https://www.home-assistant.io/)
- **PlatformIO.** [https://platformio.org/](https://platformio.org/)
