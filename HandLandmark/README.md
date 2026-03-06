# 🖐️ Hand Landmark Virtual Button → ESP32 LED Controller

Sistem IoT Computer Vision yang menggunakan **MediaPipe Hand Landmark** untuk mendeteksi gesture tangan sebagai **tombol virtual** yang mengontrol **LED pada ESP32** secara real-time.

## 📋 Arsitektur Sistem

```
📱 DroidCam (HP) → 💻 Python (MediaPipe + OpenCV) → 🔌 ESP32 (LED)
     Kamera           Hand Detection + UI             Hardware Control
```

- **Input**: Kamera HP via DroidCam
- **Processing**: MediaPipe Hand Landmark + OpenCV (Python)
- **Output**: ESP32 mengontrol LED via Serial/WiFi
- **Interaksi**: Arahkan ujung jari telunjuk ke tombol virtual di layar

## 🔧 Kebutuhan Hardware

| Komponen | Keterangan |
|----------|------------|
| ESP32 DevKit V1 | Mikrokontroler utama |
| LED + Resistor 220Ω | Atau gunakan built-in LED (GPIO 2) |
| Kabel USB | Untuk koneksi ESP32 ke laptop |
| HP Android | Sebagai kamera via DroidCam |

### Wiring

```
ESP32 GPIO 2 ──── [220Ω] ──── LED (+) ──── GND
     │
(atau gunakan built-in LED, tanpa wiring tambahan)
```

## 🚀 Setup & Instalasi

### 1. Setup DroidCam
1. Install **DroidCam** di HP (Google Play Store)
2. Install **DroidCam Client** di laptop: https://www.dev47apps.com/
3. Hubungkan HP via **USB** atau **WiFi**
4. Pastikan kamera muncul (biasanya sebagai `/dev/video0` atau `/dev/video1`)

### 2. Setup Python (Conda)
```bash
# Buat/aktifkan conda environment
conda create -n handlandmark python=3.10 -y
conda activate handlandmark

# Install dependencies
cd HandLandmark/python
pip install -r requirements.txt
```

### 3. Setup ESP32 (PlatformIO)
```bash
# Install PlatformIO CLI (jika belum)
pip install platformio

# Build & Upload firmware
cd HandLandmark/esp32-firmware
pio run --target upload

# (Opsional) Buka serial monitor untuk testing
pio device monitor --baud 115200
```

### 4. Konfigurasi

Edit `python/config.py` sesuai setup Anda:
```python
# Camera index (coba 0, 1, atau 2)
CAMERA_INDEX = 0

# Atau DroidCam WiFi
# CAMERA_INDEX = "http://192.168.1.100:4747/video"

# Serial port ESP32
SERIAL_PORT = "/dev/ttyUSB0"

# Mode komunikasi: "serial" atau "wifi"
COMM_MODE = "serial"
```

Untuk WiFi mode, edit juga `esp32-firmware/include/config.h`:
```cpp
#define WIFI_SSID     "NamaWiFiAnda"
#define WIFI_PASSWORD  "PasswordWiFi"
```

## ▶️ Menjalankan

```bash
# Aktifkan conda env
conda activate handlandmark

# Jalankan (mode serial - default)
cd HandLandmark/python
python main.py

# Atau dengan opsi:
python main.py --mode wifi           # WiFi WebSocket
python main.py --mode dummy          # Tanpa hardware (testing)
python main.py --camera 1            # Kamera index 1
python main.py --camera "http://192.168.1.100:4747/video"  # DroidCam WiFi
```

### Kontrol Keyboard
| Key | Fungsi |
|-----|--------|
| `Q` | Quit / Keluar |
| `M` | Switch mode (serial → WiFi → dummy) |
| `R` | Reconnect ke ESP32 |
| `D` | Toggle tampilan landmarks |
| `S` | Screenshot |

## 🎯 Cara Penggunaan

1. **Jalankan** aplikasi Python
2. **Posisikan** tangan di depan kamera DroidCam
3. **Arahkan** ujung jari telunjuk ke area tombol virtual (kotak merah/hijau di layar)
4. **Tahan** jari di area tombol → LED akan **toggle** ON/OFF
5. **Status bar** di bawah menampilkan: LED state, hand detection, koneksi ESP32, FPS

## 📂 Struktur Project

```
HandLandmark/
├── python/
│   ├── main.py              # Entry point
│   ├── config.py             # Konfigurasi
│   ├── hand_detector.py      # MediaPipe wrapper
│   ├── virtual_button.py     # Virtual button UI
│   ├── esp32_comm.py         # Serial/WiFi communication
│   └── requirements.txt      # Dependencies
│
├── esp32-firmware/
│   ├── platformio.ini        # PlatformIO config
│   ├── src/main.cpp          # ESP32 firmware
│   └── include/config.h      # Hardware config
│
└── README.md
```

## 🔍 Troubleshooting

| Masalah | Solusi |
|---------|--------|
| Kamera tidak terbuka | Coba camera index 0, 1, 2. Pastikan DroidCam aktif |
| Serial port error | Cek `ls /dev/ttyUSB*` atau `ls /dev/ttyACM*`. Tambahkan user ke group `dialout`: `sudo usermod -aG dialout $USER` |
| MediaPipe error | Pastikan Python 3.8-3.11. Coba `pip install mediapipe --upgrade` |
| LED tidak berkedip | Cek wiring, pastikan LED dan resistor terpasang benar |
| WiFi gagal | Pastikan ESP32 dan laptop di jaringan WiFi yang sama |
| Tombol tidak responsif | Pastikan ujung jari telunjuk benar-benar masuk ke area tombol |
