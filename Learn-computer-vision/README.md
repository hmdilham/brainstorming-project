# 🎯 Learning Path: Computer Vision dengan ESP32

## Pendahuluan

Kurikulum ini dirancang untuk mempelajari **Computer Vision (CV)** secara bertahap menggunakan pendekatan **dual-camera**:
- **Webcam Laptop** — untuk belajar dan prototyping cepat (tanpa perlu ESP32-CAM)
- **ESP32-CAM** — untuk deployment ke perangkat embedded

Pengolahan dilakukan di sisi PC menggunakan **Python (OpenCV, MediaPipe, YOLO)**, dengan **ESP32 DevKit** sebagai pengendali aktuator.

### Arsitektur Umum

```
  MODE A: Belajar/Prototyping           MODE B: Deployment/IoT
  ─────────────────────────             ──────────────────────
  ┌──────────────┐                      ┌──────────────┐     WiFi/Serial
  │   Webcam     │                      │  ESP32-CAM   │ ◄──────────────┐
  │  (Laptop)    │                      │  (OV2640)    │   Stream MJPEG │
  └──────┬───────┘                      └──────┬───────┘                │
         │ USB                                 │ WiFi                   │
  ┌──────▼───────────────┐              ┌──────▼───────────────┐        │
  │  PC (Python/OpenCV)  │              │  PC (Python/OpenCV)  │        │
  │  (Processing + AI)   │              │  (Processing + AI)   │        │
  └──────┬───────────────┘              └──────┬───────────────┘        │
         │ Serial/WiFi                         │ Serial/WiFi            │
  ┌──────▼───────────┐                  ┌──────▼───────────┐            │
  │  ESP32 DevKit    │                  │  ESP32 DevKit    │────────────┘
  │  (Aktuator)      │                  │  (Aktuator)      │
  └──────────────────┘                  └──────────────────┘
```

> **Prinsip**: Setiap sesi menyediakan contoh kode untuk **webcam** (belajar cepat) DAN **ESP32-CAM** (deployment IoT). Mulai belajar dengan webcam, lalu migrasikan ke ESP32-CAM saat siap.

---

## 📋 Daftar Project Sederhana

### Project dari Ide Anda
| # | Project | Teknik CV |
|---|---------|-----------|
| 1 | ON/OFF Lampu dengan Telapak Tangan | Hand Detection + Gesture Recognition |
| 2 | Tombol Virtual di Layar Komputer | Hand Landmark + Virtual Touch Zone |
| 3 | Belokkan Mobil RC dengan Gesture | Hand Gesture → Serial/WiFi → Motor Driver |
| 4 | Game Hindari Rintangan dengan Gesture | Pose/Hand Tracking + Game Logic (Pygame) |
| 5 | Buka Kunci dengan Face Recognition | Face Detection + Face Embedding Match |
| 6 | Pilah Barang Berdasarkan Warna | Color Detection + HSV Masking → Servo Sorter |

### Tambahan Project Sederhana Lainnya
| # | Project | Teknik CV |
|---|---------|-----------|
| 7 | **Smart Attendance (Absensi Otomatis)** | Face Recognition + Database logging |
| 8 | **People Counter (Penghitung Pengunjung)** | Object Detection (YOLO) + Counting Logic |
| 9 | **Deteksi Helm / APD Safety** | Object Classification + YOLO Custom Model |
| 10 | **Pet Feeder Otomatis** | Detect pet presence → trigger servo feeder |
| 11 | **Pendeteksi Kantuk (Drowsiness)** | Eye Aspect Ratio (EAR) + Face Landmark |
| 12 | **QR Code / Barcode Reader** | Decode QR/Barcode dari stream kamera |
| 13 | **Follow-Me Robot** | Color/Object Tracking → motor control |
| 14 | **Smart Parking (Deteksi Slot Kosong)** | Background Subtraction + contour analysis |
| 15 | **Anomaly Detection: Intrusion Alert** | Motion Detection + frame differencing → buzzer |
| 16 | **Finger Counter (Hitung Jari)** | Hand Landmark + finger state logic |
| 17 | **Anomaly Kelistrikan (Pencegah Kebakaran)** | Thermal color detection + sensor fusion (CV + arus/suhu) → auto-cutoff relay |

---

## 🗺️ Learning Path (Silabus)

Silabus disusun dalam **5 Fase** dengan total **28 sesi**, masing-masing ~2-3 jam.

---

### FASE 1: Fondasi (Sesi 1–5)
> **Tujuan**: Memahami dasar pengolahan citra, setup tools, dan koneksi ESP32-CAM.

#### Sesi 1 — Pengenalan Computer Vision & Setup Environment
- **Teori**: Apa itu Computer Vision? Perbedaan Image Processing vs CV vs AI Vision
- **Praktik**:
  - Install Python, OpenCV, pip dependencies
  - Install PlatformIO / Arduino IDE untuk ESP32
  - Hello World: Baca & tampilkan gambar dengan OpenCV (`cv2.imread`, `cv2.imshow`)
  - Manipulasi dasar: resize, crop, rotate, flip
- **Output**: Environment siap, bisa membaca dan menampilkan gambar

#### Sesi 2 — Dasar Pengolahan Citra (Image Processing)
- **Teori**: Color spaces (BGR, RGB, HSV, Grayscale), Thresholding, Filtering
- **Praktik**:
  - Konversi warna: `cv2.cvtColor()`
  - Thresholding: `cv2.threshold()`, `cv2.adaptiveThreshold()`
  - Filter: Blur (Gaussian, Median), Edge Detection (Canny)
  - Morphological Operations: Erode, Dilate, Opening, Closing
- **Output**: Bisa mengolah gambar dasar dan mendeteksi tepi

#### Sesi 3 — Contour, Shape Detection & Color Detection
- **Teori**: Contour hierarchy, moments, bounding box, HSV color range
- **Praktik**:
  - `cv2.findContours()` dan `cv2.drawContours()`
  - Deteksi bentuk (lingkaran, persegi, segitiga) berdasarkan jumlah vertex
  - **Mini Project**: Deteksi objek berdasarkan warna (HSV masking)
  - Membuat trackbar untuk tuning HSV range secara real-time
- **Output**: Bisa mendeteksi bentuk dan warna dari gambar/video

#### Sesi 4 — Setup ESP32-CAM & Video Streaming
- **Teori**: Arsitektur ESP32-CAM (OV2640), MJPEG streaming, resolusi vs FPS
- **Praktik**:
  - Flash firmware CameraWebServer ke ESP32-CAM
  - Akses stream dari browser (`http://<IP>/stream`)
  - Capture stream dari Python: `cv2.VideoCapture("http://<IP>/stream")`
  - Proses frame secara real-time (grayscale, edge detection)
- **Output**: Python bisa membaca video stream dari ESP32-CAM

#### Sesi 5 — Komunikasi ESP32-CAM ↔ ESP32 DevKit
- **Teori**: Serial UART, WiFi (HTTP/WebSocket), protokol kontrol
- **Praktik**:
  - Koneksi UART antara ESP32-CAM dan ESP32 DevKit
  - Kirim perintah dari PC → ESP32 DevKit via Serial/WiFi
  - ESP32 DevKit mengontrol LED/Relay berdasarkan perintah
  - **Mini Project**: PC deteksi warna → kirim "ON"/"OFF" ke ESP32 → LED menyala/mati
- **Output**: Pipeline end-to-end: Kamera → PC → ESP32 → Aktuator

---

### FASE 2: Deteksi & Tracking (Sesi 6–10)
> **Tujuan**: Menguasai teknik deteksi objek, wajah, dan motion.

#### Sesi 6 — Motion Detection & Background Subtraction
- **Teori**: Frame differencing, Background subtractor (MOG2, KNN)
- **Praktik**:
  - Implementasi frame differencing sederhana
  - `cv2.createBackgroundSubtractorMOG2()`
  - Deteksi gerakan dan bounding box pada area yang bergerak
  - **Mini Project 🔨**: *Intrusion Alert* — Deteksi gerakan dari ESP32-CAM → buzzer di ESP32 DevKit
- **Output**: Sistem deteksi gerak sederhana

#### Sesi 7 — Face Detection (Haar Cascade & DNN)
- **Teori**: Haar Cascade classifier, DNN-based face detector, perbandingan akurasi
- **Praktik**:
  - `cv2.CascadeClassifier()` untuk face detection
  - DNN face detector (`cv2.dnn.readNetFromCaffe`)
  - Deteksi wajah dari stream ESP32-CAM secara real-time
  - Crop dan simpan wajah yang terdeteksi
- **Output**: Bisa mendeteksi wajah secara real-time dari kamera

#### Sesi 8 — Face Recognition (Pengenalan Wajah)
- **Teori**: Face embedding, face_recognition library, threshold matching
- **Praktik**:
  - Enroll wajah (capture beberapa sample)
  - Encode wajah menjadi 128-d embedding
  - Bandingkan wajah baru dengan database
  - **Mini Project 🔨**: *Face Unlock* — Wajah dikenali → kirim perintah ke ESP32 → Servo membuka kunci
- **Output**: Sistem face recognition dengan aksi fisik (buka kunci)

#### Sesi 9 — Object Detection dengan YOLO
- **Teori**: YOLO architecture overview, YOLOv8/v11-nano, inference vs training
- **Praktik**:
  - Setup Ultralytics YOLOv8
  - Deteksi objek dari stream ESP32-CAM menggunakan pre-trained model
  - Filter objek tertentu (person, car, bottle, dll)
  - **Mini Project 🔨**: *People Counter* — Hitung jumlah orang di frame → tampilkan di OLED ESP32
- **Output**: Object detection berjalan real-time dari ESP32-CAM stream

#### Sesi 10 — Object Tracking & Counting
- **Teori**: Centroid tracking, SORT algorithm, tracking vs detection
- **Praktik**:
  - Implementasi centroid tracker sederhana
  - Track objek yang melintas garis virtual (line crossing)
  - **Mini Project 🔨**: *Smart Parking* — Deteksi slot kosong/terisi → tampilkan status di dashboard web
- **Output**: Bisa tracking dan menghitung objek yang bergerak

---

### FASE 3: Gesture & Interaksi (Sesi 11–15)
> **Tujuan**: Menguasai hand/pose tracking dan kontrol perangkat/game dengan gesture.

#### Sesi 11 — Hand Detection dengan MediaPipe
- **Teori**: MediaPipe Hands, 21 landmark points, hand region detection
- **Praktik**:
  - Setup MediaPipe: `mp.solutions.hands`
  - Deteksi tangan dan gambar landmark di frame
  - Hitung jari yang terbuka (Finger Counter logic)
  - **Mini Project 🔨**: *Finger Counter* — Jumlah jari ditampilkan di layar + dikirim ke ESP32 OLED
- **Output**: Deteksi tangan dan hitung jari real-time

#### Sesi 12 — Gesture Recognition untuk Kontrol Perangkat
- **Teori**: Gesture classification dari landmark, mapping gesture → aksi
- **Praktik**:
  - Definisikan gesture: Telapak terbuka, kepalan, peace sign, thumbs up/down
  - Klasifikasi gesture berdasarkan posisi relatif landmark
  - **Mini Project 🔨**: *ON/OFF Lampu* — Telapak terbuka = ON, Kepalan = OFF → relay ESP32
- **Output**: Kontrol lampu/relay dengan gesture tangan

#### Sesi 13 — Gesture untuk Kontrol Arah (Steering)
- **Teori**: Directional gesture, tilt detection, mapping ke aksi kontinu
- **Praktik**:
  - Deteksi arah tangan (kiri, kanan, atas, bawah) dari landmark wrist & fingertip
  - Kirim perintah steering ke ESP32 via WiFi
  - **Mini Project 🔨**: *RC Car Steering* — Gesture kiri/kanan → ESP32 → Motor driver L298N → Mobil RC belok
- **Output**: Kontrol mobil RC dengan gesture tangan

#### Sesi 14 — Virtual Touch & Tombol Virtual
- **Teori**: Virtual button zone, collision detection 2D, UI overlay
- **Praktik**:
  - Buat tombol virtual di overlay video (rectangle zones)
  - Deteksi jari telunjuk menyentuh zona tombol (fingertip within bounding box)
  - **Mini Project 🔨**: *Virtual Keyboard/Button* — Tombol virtual di layar untuk ON/OFF perangkat → kirim ke ESP32
- **Output**: Interaksi tombol virtual di kamera dengan aksi nyata

#### Sesi 15 — Gesture-Controlled Game
- **Teori**: Game loop (Pygame), collision detection, mapping gesture → game input
- **Praktik**:
  - Buat game Pygame sederhana: karakter menghindari rintangan (obstacle falling)
  - Kontrol karakter dengan gesture tangan (kiri/kanan) dari kamera
  - Tambahkan scoring dan game over logic
  - **Mini Project 🔨**: *Gesture Dodge Game* — Gerakkan tangan untuk menghindari rintangan jatuh
- **Output**: Game interaktif yang dikontrol gesture

---

### FASE 4: Anomaly Detection & Proyek Akhir (Sesi 16–20)
> **Tujuan**: Menerapkan anomaly detection, custom training, dan integrasi proyek akhir.

#### Sesi 16 — Color-Based Sorting (Penyortiran Warna)
- **Teori**: HSV masking lanjutan, multi-color detection, decision logic
- **Praktik**:
  - Deteksi warna dominan objek di conveyor area
  - Klasifikasi: Merah → Bin A, Biru → Bin B, Hijau → Bin C
  - **Mini Project 🔨**: *Color Sorter* — Kamera deteksi warna → ESP32 → Servo arahkan ke bin yang sesuai
- **Output**: Sistem penyortiran otomatis berdasarkan warna

#### Sesi 17 — Anomaly Detection: Visual Inspection
- **Teori**: Template matching, SSIM comparison, anomaly scoring
- **Praktik**:
  - Bandingkan gambar produk dengan template "good" menggunakan SSIM
  - Hitung anomaly score, set threshold
  - **Mini Project 🔨**: *Defect Detector* — Bandingkan objek di conveyor dengan referensi → reject jika cacat
- **Output**: Sistem visual inspection sederhana

#### Sesi 18 — Anomaly Detection: Behavioral (Pose Estimation)
- **Teori**: MediaPipe Pose, 33 pose landmarks, pose classification
- **Praktik**:
  - Deteksi pose tubuh (berdiri, duduk, jatuh)
  - Klasifikasi perilaku tidak normal (orang jatuh/terlentang)
  - **Mini Project 🔨**: *Fall Detection Alert* — Deteksi orang jatuh → buzzer + notifikasi ke ESP32
- **Output**: Sistem deteksi jatuh (anomali perilaku)

#### Sesi 19 — Custom Model Training (YOLOv8)
- **Teori**: Dataset preparation, labeling (Roboflow/LabelImg), transfer learning
- **Praktik**:
  - Kumpulkan dataset custom (misal: helm, objek spesifik)
  - Labeling dengan Roboflow
  - Training YOLOv8-nano dengan dataset custom
  - Deploy dan test dengan stream ESP32-CAM
  - **Mini Project 🔨**: *Helmet Detector* — Deteksi apakah pekerja memakai helm → alert jika tidak
- **Output**: Custom YOLO model yang bisa mendeteksi objek spesifik

#### Sesi 20 — Proyek Akhir: Sistem Terintegrasi
- **Teori**: System architecture, reliability, error handling
- **Praktik** (Pilih salah satu atau kombinasi):
  - **Opsi A**: *Smart Security System* — Face recognition + motion detection + intrusion alert + log ke database
  - **Opsi B**: *Smart Factory Line* — Color sorting + defect detection + counting + dashboard web
  - **Opsi C**: *Gesture-Controlled Smart Home* — Gesture recognition + virtual buttons + MQTT + multiple devices
- **Deliverables**:
  - Sistem berfungsi end-to-end
  - Dokumentasi teknis
  - Video demo
- **Output**: Proyek akhir terintegrasi yang menggabungkan minimal 3 teknik CV

---

### FASE 5: Edge AI & TinyML (Sesi 21–28)
> **Tujuan**: Memahami Edge Computing, menjalankan ML model langsung di ESP32 tanpa perlu PC, menggunakan Edge Impulse untuk rapid prototyping.

#### Sesi 21 — Pengenalan Edge Computing & TinyML
- **Teori**: Cloud vs Edge vs TinyML, mengapa inference di device penting, constraint ESP32 (RAM, Flash, CPU)
- **Praktik**:
  - Visualisasi pipeline: Cloud AI vs Edge AI vs TinyML
  - Benchmark: latency cloud vs local processing
  - Survey arsitektur TinyML: TensorFlow Lite Micro, Edge Impulse, ONNX Micro
  - Instalasi tools dasar dan environment check
- **Output**: Memahami kapan menggunakan Edge AI vs Cloud AI

#### Sesi 22 — Dasar Machine Learning untuk Pemula
- **Teori**: Supervised vs Unsupervised, Training vs Inference, Dataset/Label/Model/Accuracy
- **Praktik**:
  - Buat classifier sederhana dengan scikit-learn (iris dataset)
  - Visualisasi training process: epoch, loss, accuracy
  - Overfitting vs underfitting — demonstration
  - Konversi model ke TFLite format
- **Output**: Memahami workflow ML dari data hingga model siap deploy

#### Sesi 23 — Edge Impulse: Setup & Data Collection
- **Teori**: Platform Edge Impulse, workflow end-to-end, koneksi device
- **Praktik**:
  - Buat akun Edge Impulse, buat project baru
  - Koneksi ESP32 ke Edge Impulse (via Serial daemon / WiFi)
  - Collect data dari sensor (akselerometer, mikrofon, kamera)
  - Data explorer, splitting, augmentasi
- **Output**: Dataset siap training di Edge Impulse

#### Sesi 24 — Edge Impulse: Audio Classification (Keyword Spotting)
- **Teori**: Audio features (MFCC, Spectrogram), neural network untuk audio
- **Praktik**:
  - Collect audio samples: "nyala", "mati", "noise"
  - Design impulse: MFCC → Neural Network Classifier
  - Training, evaluasi confusion matrix
  - Deploy ke ESP32 (Arduino library export)
  - **Mini Project 🔨**: *Voice-Controlled LED* — "nyala" = LED ON, "mati" = LED OFF, tanpa internet!
- **Output**: Keyword spotting berjalan on-device di ESP32

#### Sesi 25 — Edge Impulse: Motion/Gesture Classification
- **Teori**: IMU data (accelerometer, gyroscope), time-series classification
- **Praktik**:
  - Koneksi MPU6050 ke ESP32, collect gesture data
  - Gesture: circle, wave, shake, idle
  - Design impulse: Spectral Analysis → Neural Network
  - Training dan deploy ke ESP32
  - **Mini Project 🔨**: *Magic Wand* — Gesture "circle" = nyalakan lampu, "wave" = matikan
- **Output**: Gesture recognition dari akselerometer, 100% on-device

#### Sesi 26 — Edge Impulse: Image Classification (On-Device)
- **Teori**: Transfer learning untuk image, MobileNet, quantization
- **Praktik**:
  - Collect gambar dari ESP32-CAM via Edge Impulse
  - Klasifikasi objek sederhana (3-5 kelas)
  - Transfer learning dengan MobileNetV2
  - Quantization INT8, deploy ke ESP32
  - **Mini Project 🔨**: *Smart Trash Bin* — Klasifikasi sampah (organik/anorganik) → servo buka tutup otomatis
- **Output**: Image classification berjalan di ESP32 tanpa PC

#### Sesi 27 — TensorFlow Lite Micro: Deploy Manual
- **Teori**: TFLite Micro runtime, model conversion pipeline, memory optimization
- **Praktik**:
  - Convert model Keras/TF ke TFLite (.tflite)
  - Quantization: Float32 → INT8
  - Embed model di ESP32 firmware (xxd → C array)
  - Inference loop di ESP32, ukur latency dan akurasi
  - **Mini Project 🔨**: *On-Device Anomaly Detector* — Deteksi anomali getaran mesin dari akselerometer
- **Output**: Bisa deploy model TFLite manual tanpa Edge Impulse

#### Sesi 28 — Proyek Akhir Edge AI
- **Teori**: System design, power management, optimasi model, field deployment
- **Praktik** (Pilih salah satu):
  - **Opsi A**: *Standalone Smart Doorbell* — Face detection + keyword "buka" → servo, 100% on-device
  - **Opsi B**: *Predictive Maintenance* — Klasifikasi getaran mesin (normal/abnormal) → alert
  - **Opsi C**: *Edge Vision Counter* — Klasifikasi dan hitung objek di conveyor tanpa PC
- **Deliverables**: Sistem on-device, tanpa PC, dokumentasi, video demo
- **Output**: Proyek Edge AI mandiri yang berjalan sepenuhnya di ESP32

---

## 🛠️ Kebutuhan Hardware & Software

### Hardware
| Komponen | Keterangan |
|----------|------------|
| ESP32 DevKit V1 | Mikrokontroler utama (aktuator) |
| ESP32-CAM (AI-Thinker) | Modul kamera OV2640 |
| USB-TTL (FTDI/CP2102) | Untuk flash ESP32-CAM |
| LED, Relay Module | Output kontrol |
| Servo SG90 (2-3 buah) | Sorting, door lock |
| Motor Driver L298N | Untuk RC Car |
| Motor DC (2 buah) | Roda RC Car |
| Buzzer | Alert/alarm |
| OLED SSD1306 (0.96") | Display status |
| MPU6050 (Fase 5) | Accelerometer/Gyroscope untuk TinyML gesture |
| INMP441 / MAX9814 (Fase 5) | Mikrofon untuk keyword spotting |
| Breadboard, Jumper | Prototyping |

### Software
| Software | Kegunaan |
|----------|----------|
| Python 3.10+ | Bahasa utama CV processing |
| OpenCV (`opencv-python`) | Image processing & CV |
| MediaPipe | Hand/Pose/Face detection |
| Ultralytics (`ultralytics`) | YOLOv8 object detection |
| face_recognition | Face recognition |
| Pygame | Game development |
| NumPy, imutils | Utilitas array & image |
| PlatformIO / Arduino IDE | Program ESP32 |
| Roboflow / LabelImg | Dataset labeling |
| Edge Impulse CLI (Fase 5) | TinyML platform |
| TensorFlow / TFLite (Fase 5) | Model conversion & training |
| scikit-learn (Fase 5) | ML dasar |

---

## 📊 Peta Skill per Project

```
                    ┌──────────────────────────────────────────────────────────────────┐
                    │                    SKILL PROGRESSION MAP                         │
                    └──────────────────────────────────────────────────────────────────┘

  FASE 1             FASE 2              FASE 3              FASE 4             FASE 5
  Fondasi            Deteksi             Gesture              Advanced           Edge AI
  ─────────          ──────────          ──────────           ──────────         ──────────
  OpenCV Basics ───► Motion Det. ──────► Hand Detect ───────► Anomaly Det. ───► Edge Computing
  Color Space   ───► Face Detect ──────► Gesture Recog ────► Custom YOLO  ───► TinyML Basics
  ESP32-CAM     ───► Face Recog  ──────► Virtual Touch ────► System Integ ───► Edge Impulse
  Komunikasi    ───► YOLO        ──────► Game Control  ────► Proyek Akhir ───► TFLite Micro
                     Tracking                                                  On-Device AI
```

### Mapping Project ke Sesi

| Project | Sesi Terkait | Prerequisite |
|---------|:------------:|:------------:|
| ON/OFF Lampu via Gesture | **12** | 1-5, 11 |
| Tombol Virtual di Layar | **14** | 1-5, 11-12 |
| RC Car via Gesture | **13** | 1-5, 11-12 |
| Game Hindari Rintangan | **15** | 1-5, 11-12 |
| Face Unlock | **8** | 1-5, 7 |
| Pilah Barang by Warna | **16** | 1-5, 3 |
| People Counter | **9-10** | 1-5 |
| Intrusion Alert | **6** | 1-5 |
| Drowsiness Detection | **18** | 1-5, 7, 11 |
| Helmet Detection | **19** | 1-5, 9 |
| Fall Detection | **18** | 1-5, 11 |
| Anomaly Kelistrikan | **17** | 1-5, 3, 6 |
| Voice-Controlled LED | **24** | 21-23 |
| Magic Wand Gesture | **25** | 21-23 |
| Smart Trash Bin (On-Device) | **26** | 21-23 |
| On-Device Anomaly Detector | **27** | 21-22 |
| Standalone Smart Doorbell | **28** | 21-27 |

---

## 📅 Estimasi Waktu

| Fase | Sesi | Durasi/Sesi | Total |
|:----:|:----:|:-----------:|:-----:|
| 1 - Fondasi | 5 sesi | 2-3 jam | 10-15 jam |
| 2 - Deteksi | 5 sesi | 2-3 jam | 10-15 jam |
| 3 - Gesture | 5 sesi | 2-3 jam | 10-15 jam |
| 4 - Advanced | 5 sesi | 3-4 jam | 15-20 jam |
| 5 - Edge AI | 8 sesi | 3-4 jam | 24-32 jam |
| **Total** | **28 sesi** | | **69-97 jam** |

> Dengan belajar **2-3 sesi per minggu**, seluruh learning path bisa diselesaikan dalam **~10-14 minggu**.

---

## 💡 Tips Belajar

1. **Jangan skip Fase 1** — fondasi image processing sangat penting
2. **Selalu test dengan webcam dulu** sebelum pakai ESP32-CAM (lebih mudah debug)
3. **Catat HSV range** untuk setiap warna yang sering dipakai
4. **Simpan kode modular** — buat library helper untuk fungsi yang sering dipakai
5. **Dokumentasikan setiap project** — screenshot, video, catatan error & solusi
6. **Mulai dari yang paling menarik** di setiap fase untuk menjaga motivasi
