# Sesi 23 — Edge Impulse: Setup & Data Collection

## 🎯 Tujuan Pembelajaran
- Memahami platform Edge Impulse dan workflow-nya
- Membuat akun dan project baru
- Menghubungkan ESP32 ke Edge Impulse
- Mengumpulkan data sensor langsung dari ESP32
- Memahami data explorer, splitting, dan augmentasi

---

## 📖 Teori

### Apa itu Edge Impulse?

Edge Impulse = **platform online** untuk membuat model TinyML **tanpa perlu coding ML dari nol**. Bayangkan seperti "Website builder, tapi untuk AI."

```
┌─────────────────────────────────────────────────────────────────┐
│                    EDGE IMPULSE WORKFLOW                         │
│                                                                 │
│   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐  │
│   │ 1. DATA  │──►│2. IMPULSE│──►│3. TRAIN  │──►│4. DEPLOY │  │
│   │ Collect  │   │ Design   │   │ & Test   │   │ to ESP32 │  │
│   └──────────┘   └──────────┘   └──────────┘   └──────────┘  │
│                                                                 │
│   Kumpulkan      Pilih fitur     Training      Download         │
│   data dari      extraction &    otomatis,     Arduino lib      │
│   sensor/kamera  model type      lihat hasil   / C++ file       │
│                                                                 │
│   🎤 Audio       MFCC            Neural Net    .zip library     │
│   📷 Image       Transfer Learn  CNN           .ino ready!      │
│   📊 Sensor      Spectral Anal   Dense NN                       │
└─────────────────────────────────────────────────────────────────┘
```

### Kenapa Edge Impulse?

```
❌ TANPA Edge Impulse:
   1. Tulis kode Python untuk collect data
   2. Tulis kode preprocessing (MFCC, FFT, dll)
   3. Tulis kode model (Keras/TF)
   4. Training di PC
   5. Convert ke TFLite
   6. Convert ke C array (xxd)
   7. Tulis kode inference ESP32
   8. Debug memory issues
   → 🕒 Berminggu-minggu belajar + trial error

✅ DENGAN Edge Impulse:
   1. Collect data (via browser/CLI)
   2. Design impulse (klik-klik di web)
   3. Train (tombol "Start training")
   4. Deploy (download Arduino library, copy-paste)
   → 🕒 Beberapa jam saja!
```

### Cara Edge Impulse Berkomunikasi dengan ESP32

```
┌───────────────────────────────────────────────────┐
│                                                   │
│  ESP32 ──(USB)──► edge-impulse-daemon ──(internet)──► Edge Impulse Cloud
│                   (program di PC)                        (website)
│                                                   │
│  Atau:                                            │
│  ESP32 ──(WiFi)──► Edge Impulse Cloud              │
│                   (direct, tanpa PC setelah setup)│
│                                                   │
└───────────────────────────────────────────────────┘

Langkah:
1. Flash firmware khusus Edge Impulse ke ESP32
2. Jalankan `edge-impulse-daemon` di PC
3. Daemon jadi "jembatan" antara ESP32 dan cloud
4. Collect data langsung dari browser Edge Impulse
```

---

## 🛠️ Praktik

### 1. Buat Akun Edge Impulse

```
1. Buka https://studio.edgeimpulse.com/
2. Klik "Sign up" → buat akun gratis
3. Setelah login → klik "Create new project"
4. Beri nama: "ESP32-TinyML-Test"
5. Pilih tipe: sesuai kebutuhan nanti
```

### 2. Install Edge Impulse CLI

```bash
# Pastikan Node.js sudah terinstall
node --version    # Harus v14 atau lebih baru

# Install Edge Impulse CLI
npm install -g edge-impulse-cli

# Verifikasi instalasi
edge-impulse-daemon --version

# Tools lain yang terinstall:
# - edge-impulse-daemon       → koneksi device
# - edge-impulse-data-forwarder → kirim data sensor
# - edge-impulse-uploader     → upload file
```

### 3. [ESP32] Firmware: Data Forwarder

Cara termudah menghubungkan ESP32 ke Edge Impulse adalah menggunakan **Data Forwarder**. ESP32 kirim data sensor via Serial, program `edge-impulse-data-forwarder` di PC meneruskan ke cloud.

```cpp
/**
 * sesi23_data_forwarder.ino
 * ESP32 kirim data sensor ke Edge Impulse via Data Forwarder
 *
 * Contoh ini menggunakan analog sensor (misal potensiometer)
 * sebagai demo sederhana. Nanti bisa diganti MPU6050, mic, dll.
 *
 * WIRING:
 *  - Potensiometer center → GPIO 34 (analog)
 *  - Atau sensor apapun yang menghasilkan data analog
 */

#define SENSOR_PIN_1  34    // Analog input 1
#define SENSOR_PIN_2  35    // Analog input 2 (opsional)

// Sampling rate: 50 Hz (20ms interval)
#define INTERVAL_MS   20
unsigned long lastSample = 0;

void setup() {
    Serial.begin(115200);
    // Tunggu serial ready
    while (!Serial);
    delay(1000);
}

void loop() {
    unsigned long now = millis();

    if (now - lastSample >= INTERVAL_MS) {
        lastSample = now;

        // Baca sensor
        float val1 = analogRead(SENSOR_PIN_1);
        float val2 = analogRead(SENSOR_PIN_2);

        // Normalize ke 0-1 (opsional tapi direkomendasikan)
        float norm1 = val1 / 4095.0;
        float norm2 = val2 / 4095.0;

        // Kirim format CSV: data1,data2
        // Data Forwarder akan parsing format ini otomatis
        Serial.printf("%.4f,%.4f\n", norm1, norm2);
    }
}
```

### 4. Koneksi ESP32 ke Edge Impulse

```bash
# === METODE 1: Data Forwarder (Sensor Data) ===

# Setelah flash firmware di atas ke ESP32, jalankan:
edge-impulse-data-forwarder

# Akan muncul:
#   ? What is your user name or e-mail address?  → masukkan email
#   ? What is your password?                      → masukkan password
#   ? To which project do you want to connect?    → pilih project
#   ? What name do you want to give this device?  → misal "esp32-devkit"
#   ? How many sensors values does your device send? → 2 (sesuai kode)
#   ? What labels do you want to give each value? → "sensor1,sensor2"
#
# Setelah connected, buka Edge Impulse Studio → Data acquisition
# → klik "Start sampling" → data ESP32 akan masuk!


# === METODE 2: Flash Firmware Edge Impulse Resmi ===
# (Untuk ESP32-CAM / ESP32 dengan dukungan resmi)

# 1. Buka: https://docs.edgeimpulse.com/docs/development-platforms/officially-supported-mcu-targets
# 2. Cari "ESP32" → download firmware
# 3. Flash firmware ke ESP32-CAM
# 4. Jalankan:
edge-impulse-daemon
# → Otomatis detect ESP32 dan connect ke project
```

### 5. Collect Data via Browser

```python
"""
sesi23_data_collection_guide.py
Panduan step-by-step mengumpulkan data di Edge Impulse

Jalankan ini untuk melihat panduan di terminal.
"""

guide = """
╔═══════════════════════════════════════════════════════════════╗
║           PANDUAN COLLECT DATA DI EDGE IMPULSE                ║
╠═══════════════════════════════════════════════════════════════╣
║                                                               ║
║  STEP 1: Buka Edge Impulse Studio                            ║
║          https://studio.edgeimpulse.com/ → project Anda       ║
║                                                               ║
║  STEP 2: Klik menu "Data acquisition"                        ║
║                                                               ║
║  STEP 3: Pilih device yang sudah terkoneksi                  ║
║          (nama device yang Anda set tadi)                     ║
║                                                               ║
║  STEP 4: Atur parameter:                                     ║
║          - Label: nama kategori (misal "idle", "wave")        ║
║          - Sample length: durasi (misal 2000 ms)              ║
║          - Frequency: sampling rate (misal 50 Hz)             ║
║                                                               ║
║  STEP 5: Klik "Start sampling"                               ║
║          → Data dari ESP32 akan direkam                       ║
║                                                               ║
║  STEP 6: Ulangi untuk setiap label/kategori                  ║
║          Minimal 50 sample per label (lebih banyak = lebih   ║
║          bagus)                                               ║
║                                                               ║
║  STEP 7: Review di "Data explorer"                           ║
║          → Pastikan data tiap kelas terlihat berbeda          ║
║                                                               ║
║  TIPS:                                                        ║
║  ✅ Variasikan kondisi saat collect (posisi, kecepatan)      ║
║  ✅ Collect noise/idle sebagai kelas "lainnya"               ║
║  ✅ Minimal 2 menit data per kelas untuk hasil bagus         ║
║  ❌ Jangan collect di lingkungan yang terlalu terkontrol     ║
║     (model harus bisa kerja di kondisi nyata!)               ║
║                                                               ║
╚═══════════════════════════════════════════════════════════════╝

AUTO SPLIT:
  Edge Impulse otomatis split data menjadi:
  - Training set (80%) → untuk melatih model
  - Test set (20%)     → untuk menguji model

  Anda bisa juga manual upload file ke training/test set

AUGMENTASI:
  Jika data masih sedikit, gunakan augmentasi:
  - Noise injection → tambah noise random
  - Time shift      → geser waktu sedikit
  - Flip            → balik data
  Edge Impulse bisa melakukan ini otomatis via DSP block
"""

print(guide)
```

### 6. [OPSIONAL] Upload Data dari File

```python
"""
sesi23_upload_data.py
Upload data dari file CSV ke Edge Impulse menggunakan CLI

Berguna jika Anda sudah punya dataset tersimpan di file.
"""
import csv
import os
import json

# Buat sample data CSV (simulasi data sensor)
import numpy as np

SAVE_DIR = "ei_upload_data"
os.makedirs(SAVE_DIR, exist_ok=True)

# Contoh: Generate data sinusoidal untuk 2 kelas
def generate_sample(label, idx, frequency=1.0, noise=0.1):
    """Generate sample data sensor 2 detik @ 50Hz."""
    t = np.linspace(0, 2, 100)  # 2 detik, 50 Hz
    if label == "normal":
        data = np.sin(2 * np.pi * frequency * t) + np.random.randn(100) * noise
    elif label == "anomaly":
        data = np.sin(2 * np.pi * frequency * 3 * t) * 2 + np.random.randn(100) * noise * 3
    else:
        data = np.random.randn(100) * noise  # noise

    # Format CBOR/JSON yang Edge Impulse terima
    sample = {
        "protected": {
            "ver": "v1",
            "alg": "none"
        },
        "signature": "0",
        "payload": {
            "device_name": "python-sim",
            "device_type": "PYTHON",
            "interval_ms": 20,
            "sensors": [
                {"name": "sensor1", "units": "m/s2"}
            ],
            "values": [[float(v)] for v in data]
        }
    }

    filename = f"{SAVE_DIR}/{label}.{idx}.json"
    with open(filename, 'w') as f:
        json.dump(sample, f)

    return filename


# Generate samples
for i in range(10):
    generate_sample("normal", i)
    generate_sample("anomaly", i)

print(f"Generated {20} sample files in {SAVE_DIR}/")
print()
print("Upload ke Edge Impulse:")
print(f"  edge-impulse-uploader --category training {SAVE_DIR}/normal.*.json --label normal")
print(f"  edge-impulse-uploader --category training {SAVE_DIR}/anomaly.*.json --label anomaly")
```

---

## 📋 Checklist Sesi Ini

| # | Task | Status |
|---|------|:------:|
| 1 | Buat akun Edge Impulse | ⬜ |
| 2 | Install Edge Impulse CLI (`npm install -g edge-impulse-cli`) | ⬜ |
| 3 | Flash Data Forwarder firmware ke ESP32 | ⬜ |
| 4 | Jalankan `edge-impulse-data-forwarder` dan connect ke project | ⬜ |
| 5 | Collect minimal 20 sample data dari browser | ⬜ |
| 6 | Review data di Data Explorer | ⬜ |

---

## 📝 Latihan Mandiri

1. **Setup lengkap**: Buat akun → install CLI → connect ESP32
2. **Collect data sederhana**: Gunakan potensiometer, rekam 2 posisi berbeda (rendah/tinggi)
3. **Upload file**: Generate data sintetis dengan script Python → upload ke Edge Impulse
4. **Explore**: Buka Data Explorer, lihat bagaimana data berbeda per kelas

---

## 🚨 Troubleshooting

| Problem | Solusi |
|---------|--------|
| `edge-impulse-daemon: command not found` | Pastikan Node.js terinstall, lalu `npm install -g edge-impulse-cli` |
| ESP32 tidak terdeteksi | Cek kabel USB, driver CP2102/CH340, coba port lain |
| Data forwarder error "invalid data" | Pastikan format output Serial: `value1,value2\n` (CSV) |
| Tidak bisa login dari CLI | Cek email/password, atau gunakan API key dari project settings |

---

## 📚 Referensi
- [Edge Impulse Getting Started](https://docs.edgeimpulse.com/docs/)
- [Data Forwarder Guide](https://docs.edgeimpulse.com/docs/tools/edge-impulse-cli/cli-data-forwarder)
- [ESP32 + Edge Impulse](https://docs.edgeimpulse.com/docs/development-platforms/officially-supported-mcu-targets/espressif-esp32)
