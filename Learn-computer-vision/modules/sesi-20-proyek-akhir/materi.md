# Sesi 20 — Proyek Akhir: Sistem Terintegrasi

## 🎯 Tujuan Pembelajaran
- Mengintegrasikan beberapa teknik CV dalam satu sistem
- Merancang arsitektur sistem end-to-end
- Membuat sistem yang reliable dan well-documented
- Deliverable: Demo berfungsi + dokumentasi teknis

---

## 📋 Pilihan Proyek Akhir

Pilih **salah satu** atau buat variasi sendiri:

---

### Opsi A: Smart Security System
**Teknik**: Face Recognition + Motion Detection + Intrusion Alert

```
                   ┌──────────────────────┐
                   │   Smart Security     │
                   └──────────┬───────────┘
           ┌──────────────────┼──────────────────┐
     ┌─────▼─────┐     ┌─────▼─────┐     ┌──────▼─────┐
     │  Motion   │     │   Face    │     │  Logging   │
     │ Detection │     │   Recog   │     │  Database  │
     └─────┬─────┘     └─────┬─────┘     └──────┬─────┘
           │                 │                   │
     ┌─────▼─────────────────▼───────────────────▼─────┐
     │              ESP32 DevKit                       │
     │  Servo(lock) + Buzzer + OLED + LED              │
     └─────────────────────────────────────────────────┘
```

**Fitur**:
- Idle: motion detection aktif (kamera ESP32-CAM / webcam)
- Motion terdeteksi → face recognition
- Wajah dikenali (authorized) → buka kunci (servo), log "ACCESS GRANTED"
- Wajah tidak dikenali → buzzer, LED merah, log "INTRUDER", simpan foto
- Dashboard: log akses via serial/web
- OLED: tampilkan status "LOCKED / UNLOCKED / INTRUDER"

---

### Opsi B: Smart Factory Line
**Teknik**: Color Sorting + Visual Inspection + Object Counting

```
     Conveyor Area
     ┌──────────────────────────────────────────┐
     │  [obj] ──► [color?] ──► [defect?] ──► [count] ──► [sort]
     └──────────────────────────────────────────┘
                       │
                 ┌─────▼─────┐
                 │  Python   │
                 │  Pipeline │
                 └─────┬─────┘
                       │
                 ┌─────▼─────┐
                 │  ESP32    │
                 │  Servo    │
                 │  OLED     │
                 └───────────┘
```

**Fitur**:
- Deteksi objek di area conveyor (kamera)
- Identifikasi warna (HSV masking)
- Visual inspection: bandingkan dengan referensi (SSIM)
- Jika OK → sort ke bin berdasarkan warna (servo)
- Jika DEFECT → reject bin
- Counter: jumlah total, per warna, defect rate
- Dashboard OLED + web

---

### Opsi C: Gesture-Controlled Smart Home
**Teknik**: Gesture Recognition + Virtual Buttons + Multi-device Control

```
     ┌──────────┐     ┌──────────────┐     ┌──────────────┐
     │  Camera  │────►│ MediaPipe    │────►│  Multi-ESP32 │
     │          │     │ Gesture+     │     │  via WiFi    │
     │          │     │ Virtual UI   │     └──────┬───┬───┘
     └──────────┘     └──────────────┘            │   │
                                            ┌─────▼┐ ┌▼─────┐
                                            │ESP32A│ │ESP32B│
                                            │Light │ │Fan   │
                                            └──────┘ └──────┘
```

**Fitur**:
- Panel virtual dengan tombol dan slider di overlay kamera
- Gesture recognition: palm=ON, fist=OFF, thumbs up=confirm
- Kontrol multiple perangkat (lampu, kipas, servo)
- Mode selector: gesture mode / virtual button mode
- Dashboard status semua perangkat

---

### Opsi D: Fire Prevention & Electrical Monitoring
**Teknik**: CV Fire Detection + Sensor Fusion + Auto-cutoff

**Fitur**:
- Kamera monitor area panel listrik
- CV detection: percikan api, asap, kabel terkelupas
- Sensor backup: ACS712 (arus), DS18B20 (suhu)
- Auto-cutoff relay jika anomaly terdeteksi
- Logging ke SD card / database
- Alert via buzzer + LED + serial notification

---

## 🏗️ Cara Mengerjakan

### 1. Arsitektur & Planning (30 menit)
```python
"""
Contoh perencanaan project — tulis di file README project
"""
# PROJECT: Smart Security System
#
# ARSITEKTUR:
#   Input:  Webcam (development) / ESP32-CAM (deployment)
#   Process: Python (OpenCV, face_recognition, MediaPipe)
#   Output:  ESP32 DevKit (Servo, Buzzer, OLED, LED)
#   Comm:    Serial UART (tethered) / WiFi HTTP (wireless)
#
# MODULES:
#   1. camera_module.py    — baca kamera (webcam/ESP32-CAM)
#   2. motion_module.py    — background subtraction
#   3. face_module.py      — face detection + recognition
#   4. esp32_comm.py       — kirim perintah ke ESP32
#   5. logger.py           — logging events ke file
#   6. main.py             — orchestrator
#
# FLOW:
#   1. Idle → motion detection
#   2. Motion detected → activate face recognition
#   3. Face recognized (authorized) → UNLOCK
#   4. Face unknown → ALARM
#   5. No motion for 30s → re-arm
```

### 2. Modular Development (2-3 jam)
Buat setiap modul secara terpisah dan test independen.

### 3. Integration (1 jam)
Gabungkan semua modul di `main.py`.

### 4. Testing & Debugging (30 menit)

### 5. Documentation & Demo Video (30 menit)

---

## 📦 Template Project Structure

```
proyek-akhir/
├── main.py                 # Entry point
├── config.py               # Konfigurasi (IP, port, threshold)
├── modules/
│   ├── __init__.py
│   ├── camera.py           # Camera handler (webcam/ESP32-CAM)
│   ├── detector.py         # Detection module
│   ├── recognizer.py       # Recognition module
│   ├── esp32.py            # ESP32 communication
│   └── logger.py           # Event logger
├── esp32_firmware/
│   ├── platformio.ini
│   └── src/
│       └── main.cpp
├── data/
│   ├── face_encodings.pkl  # Database wajah
│   └── logs/               # Event logs
├── models/
│   └── best.pt             # Custom YOLO model (jika ada)
└── README.md               # Dokumentasi
```

### Template main.py

```python
"""
main.py — Template entry point proyek akhir
"""
import cv2
import imutils
import time
import argparse

# Import modules Anda
# from modules.camera import open_camera, read_frame
# from modules.detector import MotionDetector, FireDetector
# from modules.recognizer import FaceRecognizer
# from modules.esp32 import ESP32Controller
# from modules.logger import EventLogger


def parse_args():
    parser = argparse.ArgumentParser(description="CV Final Project")
    parser.add_argument("--camera", default="0",
                        help="Camera source: 0 (webcam) or ESP32 URL")
    parser.add_argument("--esp32", default=None,
                        help="ESP32 serial port or WiFi IP")
    parser.add_argument("--mode", default="serial",
                        choices=["serial", "wifi", "demo"],
                        help="Communication mode")
    return parser.parse_args()


def main():
    args = parse_args()

    # Setup camera
    source = int(args.camera) if args.camera.isdigit() else args.camera
    cap = cv2.VideoCapture(source)

    if not cap.isOpened():
        print("ERROR: Cannot open camera!")
        return

    # Setup ESP32 (if connected)
    # esp32 = ESP32Controller(args.esp32, args.mode)

    # Setup detector/recognizer
    # motion = MotionDetector()
    # recognizer = FaceRecognizer("data/face_encodings.pkl")
    # logger = EventLogger("data/logs/")

    print("System started! Press 'q' to quit.")

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        frame = imutils.resize(frame, width=640)
        display = frame.copy()

        # ===== YOUR LOGIC HERE =====
        # Contoh flow:
        #
        # 1. Check motion
        # has_motion = motion.detect(frame)
        #
        # 2. If motion → recognize
        # if has_motion:
        #     name, confidence = recognizer.recognize(frame)
        #     if name in AUTHORIZED:
        #         esp32.send("UNLOCK")
        #         logger.log("ACCESS", name)
        #     else:
        #         esp32.send("ALARM")
        #         logger.log("INTRUDER", "unknown")

        timestamp = time.strftime("%H:%M:%S")
        cv2.putText(display, f"System Active | {timestamp}",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        cv2.imshow("Final Project", display)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
```

---

## ✅ Checklist Deliverables

| # | Item | Status |
|---|------|:------:|
| 1 | Sistem berfungsi end-to-end | ⬜ |
| 2 | Minimal 3 teknik CV terintegrasi | ⬜ |
| 3 | Dual-camera support (webcam + ESP32-CAM) | ⬜ |
| 4 | ESP32 aktuator terhubung dan berfungsi | ⬜ |
| 5 | Error handling (kamera mati, koneksi putus) | ⬜ |
| 6 | README.md dengan arsitektur & cara deploy | ⬜ |
| 7 | Video demo (2-3 menit) | ⬜ |
| 8 | Kode terorganisir (modular, bersih) | ⬜ |

---

## 🏆 Selamat!

Jika Anda sampai di sini, Anda telah menguasai:
- ✅ Image Processing dasar (OpenCV)
- ✅ Face Detection & Recognition
- ✅ Object Detection (YOLO)
- ✅ Object Tracking & Counting
- ✅ Hand/Pose Detection (MediaPipe)
- ✅ Gesture Recognition & Control
- ✅ Anomaly Detection (Visual + Behavioral)
- ✅ Custom Model Training
- ✅ Integrasi CV + IoT (ESP32)
- ✅ Dual Camera (Webcam + ESP32-CAM)

**Next steps**: Eksplorasi lebih lanjut ke Edge AI (TensorFlow Lite Micro di ESP32), advanced object tracking (DeepSORT), atau 3D pose estimation!
