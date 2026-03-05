# Sesi 28 — Proyek Akhir Edge AI

## 🎯 Tujuan Pembelajaran
- Merancang sistem Edge AI end-to-end
- Mengintegrasikan sensor, model ML, dan aktuator di ESP32
- Membuat sistem yang berjalan **100% mandiri** (tanpa PC, tanpa internet)
- Deliverable: Perangkat Edge AI yang berfungsi + dokumentasi

---

## 📋 Pilihan Proyek Akhir Edge AI

---

### Opsi A: Standalone Smart Doorbell
**Kombinasi**: Image Classification + Keyword Spotting + Servo

```
┌─────────────────────────────────────────────────────────────┐
│                 STANDALONE SMART DOORBELL                     │
│                                                             │
│  Kamera (ESP32-CAM)                                         │
│    │                                                        │
│    ▼                                                        │
│  Image Classifier ──► "Orang terdeteksi"                    │
│    │                      │                                 │
│    │                      ▼                                 │
│    │               Mikrofon INMP441                         │
│    │                      │                                 │
│    │                      ▼                                 │
│    │               Keyword Spotter                           │
│    │               "buka" detected?                          │
│    │                   │        │                            │
│    │                  YES       NO                           │
│    │                   │        │                            │
│    │               Servo OPEN  LED merah                     │
│    │               OLED: "Welcome!"  OLED: "Access Denied"  │
│    │               Buzzer: ✅        Buzzer: ❌              │
│    │                                                        │
│    └── Semua di ESP32! Tanpa PC, tanpa WiFi!                │
└─────────────────────────────────────────────────────────────┘

Komponen:
  - ESP32-CAM (kamera + processing)
  - INMP441 (mikrofon I2S)
  - Servo SG90 (kunci pintu)
  - OLED SSD1306 (display status)
  - Buzzer (alert)
  - LED merah + hijau

Model yang dibutuhkan:
  1. Image classifier: "person" vs "empty" (~50 KB)
  2. Keyword spotter: "buka" vs "noise" (~20 KB)
  Total: ~70 KB → ✅ muat di ESP32!
```

---

### Opsi B: Predictive Maintenance
**Kombinasi**: Accelerometer + Anomaly Detection + Alert System

```
┌─────────────────────────────────────────────────────────────┐
│               PREDICTIVE MAINTENANCE                         │
│                                                             │
│  MPU6050 (accelerometer)                                     │
│    │                                                        │
│    ▼                                                        │
│  Data Buffer (1 detik @ 50Hz)                                │
│    │                                                        │
│    ▼                                                        │
│  TFLite Micro Model                                         │
│    │                                                        │
│    ├── "normal"     → LED hijau, OLED: "OK"                 │
│    ├── "warning"    → LED kuning, OLED: "Check soon"        │
│    └── "critical"   → LED merah, Buzzer, OLED: "STOP!"     │
│                        + Relay OFF (matikan mesin)           │
│                                                             │
│  Logging:                                                   │
│    - Timestamp setiap anomaly                               │
│    - Counter: total jam operasi                             │
│    - Counter: total anomaly                                 │
│                                                             │
│  Semua on-device! Bisa pakai baterai!                       │
└─────────────────────────────────────────────────────────────┘

Komponen:
  - ESP32 DevKit
  - MPU6050 (accelerometer/gyroscope)
  - OLED SSD1306
  - LED (hijau, kuning, merah)
  - Buzzer
  - Relay module (opsional — untuk auto-shutoff)

Model: 3-class classifier (~30 KB)
```

---

### Opsi C: Edge Vision Counter
**Kombinasi**: Image Classification + Counting + Display

```
┌─────────────────────────────────────────────────────────────┐
│               EDGE VISION COUNTER                            │
│                                                             │
│  ESP32-CAM                                                   │
│    │                                                        │
│    ▼                                                        │
│  Ambil gambar setiap 2 detik                                │
│    │                                                        │
│    ▼                                                        │
│  Image Classifier:                                           │
│    "produk_A" / "produk_B" / "defect" / "empty"             │
│    │                                                        │
│    ├── "produk_A" → counter_A++, servo ke bin A             │
│    ├── "produk_B" → counter_B++, servo ke bin B             │
│    ├── "defect"   → counter_defect++, servo ke reject bin   │
│    └── "empty"    → tunggu produk berikutnya                │
│                                                             │
│  OLED Display:                                               │
│    ┌──────────────────────┐                                 │
│    │ Product A:  145      │                                 │
│    │ Product B:   87      │                                 │
│    │ Defects:     12      │                                 │
│    │ Rate: 98.3% OK      │                                 │
│    └──────────────────────┘                                 │
│                                                             │
│  100% offline! Cocok untuk pabrik tanpa internet             │
└─────────────────────────────────────────────────────────────┘

Model: 4-class image classifier (~100 KB)
```

---

## 🏗️ Cara Mengerjakan

### 1. Planning (30 menit)

```
CHECKLIST PLANNING:
□ Pilih opsi project (A, B, atau C)
□ Gambar diagram arsitektur sistem
□ List semua komponen hardware yang dibutuhkan
□ Tentukan model apa yang dibutuhkan (tipe, kelas, input size)
□ Tentukan bagaimana data akan di-collect
□ Tentukan aktuator apa dan logika kontrolnya
```

### 2. Data Collection (1-2 jam)

```
□ Collect data dari sensor/kamera
□ Upload ke Edge Impulse (atau siapkan dataset lokal)
□ Review data di Data Explorer
□ Pastikan minimal 50 sample per kelas
□ Variasikan kondisi pengambilan data
```

### 3. Training (30 menit)

```
□ Design impulse di Edge Impulse (atau train manual di Python)
□ Generate features
□ Training model
□ Evaluasi: accuracy > 80%?
□ Jika kurang → tambah data atau tweak model
□ Cek model size: < 200 KB?
```

### 4. Deploy & Firmware (1-2 jam)

```
□ Deploy model (Arduino library ATAU TFLite manual)
□ Wiring semua komponen
□ Tulis firmware ESP32:
  - Inisialisasi sensor
  - Inisialisasi model
  - Loop: baca data → inference → aksi
  - Display status di OLED
  - Error handling
□ Flash ke ESP32
```

### 5. Testing (30 menit)

```
□ Test inference accuracy di kondisi nyata
□ Test semua aktuator (servo, LED, buzzer)
□ Test edge cases (tidak ada objek, objek baru, noise tinggi)
□ Ukur latency inference
□ Ukur konsumsi daya (opsional)
□ Cek stability (jalankan 30+ menit tanpa crash)
```

### 6. Documentation (30 menit)

```
□ Tulis README.md:
  - Deskripsi project
  - Arsitektur diagram
  - Hardware list
  - Cara deploy/replicate
  - Hasil testing
□ Rekam video demo (2-3 menit)
□ Screenshot confusion matrix dari training
```

---

## 📦 Template Project Structure

```
proyek-akhir-edge-ai/
├── README.md                    # Dokumentasi utama
├── firmware/
│   ├── platformio.ini           # PlatformIO config
│   ├── src/
│   │   └── main.cpp             # Firmware utama
│   └── lib/
│       └── model/               # Model files (.h)
│           └── model_data.h
├── training/
│   ├── dataset/                 # Data training
│   ├── train_model.py           # Script training (jika manual)
│   ├── convert_model.py         # Script konversi TFLite
│   └── model_int8.tflite        # Model terkonversi
├── docs/
│   ├── wiring_diagram.png       # Diagram wiring
│   ├── training_results.png     # Confusion matrix
│   └── demo_video.mp4           # Video demo
└── edge_impulse/
    └── project_link.txt         # Link ke Edge Impulse project
```

---

## ✅ Checklist Deliverables

| # | Item | Status |
|---|------|:------:|
| 1 | Sistem berjalan 100% on-device (tanpa PC) | ⬜ |
| 2 | Model ML berfungsi dengan accuracy > 80% | ⬜ |
| 3 | Minimal 1 sensor input (kamera/mic/accelerometer) | ⬜ |
| 4 | Minimal 1 aktuator output (servo/LED/buzzer/relay) | ⬜ |
| 5 | OLED display menampilkan status | ⬜ |
| 6 | Inference latency < 500ms | ⬜ |
| 7 | Stabil berjalan > 30 menit | ⬜ |
| 8 | README.md lengkap | ⬜ |
| 9 | Video demo | ⬜ |
| 10 | Kode terorganisir dan terdokumentasi | ⬜ |

---

## 🏆 Selamat! Anda Telah Menyelesaikan Seluruh Learning Path!

```
╔═══════════════════════════════════════════════════════════════════╗
║                                                                   ║
║   🎓 SKILL YANG TELAH ANDA KUASAI:                              ║
║                                                                   ║
║   Fase 1: ✅ Image Processing (OpenCV)                           ║
║   Fase 2: ✅ Detection & Recognition (Face, YOLO, Tracking)      ║
║   Fase 3: ✅ Gesture Control (MediaPipe, Game, Virtual Touch)     ║
║   Fase 4: ✅ Anomaly Detection, Custom YOLO, System Integration   ║
║   Fase 5: ✅ Edge Computing, TinyML, Edge Impulse, TFLite Micro   ║
║                                                                   ║
║   Anda sekarang bisa:                                            ║
║   • Memproses gambar dan video                                    ║
║   • Mendeteksi dan mengenali objek/wajah/gesture                  ║
║   • Membuat model ML custom                                      ║
║   • Deploy AI ke mikrokontroler (ESP32)                           ║
║   • Membangun sistem IoT + AI end-to-end                         ║
║                                                                   ║
║   🚀 NEXT STEPS:                                                 ║
║   • DeepSort (advanced tracking)                                  ║
║   • YOLO on-device (ESP32-S3 dengan PSRAM)                       ║
║   • TinyML dengan sensor lain (radar, lidar)                     ║
║   • LoRa + TinyML (long range IoT + AI)                          ║
║   • Multi-camera systems                                         ║
║   • MLOps untuk embedded (versioning, monitoring)                 ║
║                                                                   ║
╚═══════════════════════════════════════════════════════════════════╝
```

---

## 📚 Referensi Lanjutan
- [TinyML Book](https://www.oreilly.com/library/view/tinyml/9781492052036/)
- [Edge Impulse Expert Projects](https://docs.edgeimpulse.com/experts/)
- [ESP32-S3 AI Features](https://www.espressif.com/en/products/socs/esp32-s3)
- [TFLite Micro Examples](https://github.com/tensorflow/tflite-micro)
- [MLOps for Edge](https://neptune.ai/blog/mlops-for-edge)
