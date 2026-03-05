# Materi IoT Lanjutan

---

### **Silabus Mata Kuliah: IoT Lanjutan — Komunikasi Data, Computer Vision & Edge AI**

**Deskripsi Mata Kuliah:**
Mata kuliah ini merupakan kelanjutan dari kursus IoT Dasar. Jika di kursus Dasar protokol komunikasi (UART, I2C, SPI) digunakan untuk komunikasi **ESP32 ↔ sensor/aktuator**, di kursus Lanjutan ini fokus berpindah ke komunikasi **antar-ESP32** dan ke **jaringan/internet**. Mahasiswa akan mendalami komunikasi fisik antar mikrokontroler, protokol jaringan modern (HTTP REST, MQTT Pub/Sub, ESP-NOW), Computer Vision berbasis kamera ESP32-CAM, serta konsep Edge AI / TinyML untuk menjalankan model kecerdasan buatan langsung di mikrokontroler. Setiap topik dirancang agar saling terhubung dan berujung pada proyek akhir sistem IoT terpadu.

> **Perbedaan kunci dengan IoT Dasar:**
> - **IoT Dasar (P5–7):** Protokol UART, I2C, SPI digunakan untuk komunikasi ESP32 dengan **sensor dan aktuator/modul** (satu ESP32 → banyak periferal).
> - **IoT Lanjutan (mulai P1):** Protokol yang **sama** kini digunakan untuk komunikasi **antar-ESP32** (dua atau lebih mikrokontroler saling bertukar data), kemudian berkembang ke komunikasi nirkabel (ESP-NOW, WiFi) dan jaringan internet (HTTP, MQTT).

**Capaian Pembelajaran:**
- Membangun komunikasi antar dua ESP32 secara fisik menggunakan UART dan I2C Master/Slave.
- Mengimplementasikan ESP32 sebagai HTTP Client untuk konsumsi REST API (GET & POST).
- Merancang dan mengimplementasikan arsitektur Pub/Sub menggunakan protokol MQTT.
- Membangun jaringan sensor nirkabel menggunakan ESP-NOW (point-to-point dan multi-node gateway).
- Mengonfigurasi ESP32-CAM untuk streaming video dan computer vision on-device.
- Menjalankan inferensi model TensorFlow Lite Micro di ESP32 (Edge AI / TinyML).
- Memproses video dari ESP32-CAM menggunakan Python + OpenCV di komputer.
- Menerapkan algoritma deteksi objek YOLO dan pengenalan gesture menggunakan MediaPipe.
- Merancang sistem IoT terpadu yang menggabungkan komunikasi data, computer vision, dan kecerdasan edge.

**Prasyarat:**
- Telah menyelesaikan kursus **IoT Dasar** (GPIO, WiFi, Bluetooth, UART, I2C, SPI, WebServer, Blynk/MQTT dasar).
- Pemahaman dasar **Python** (variabel, loop, kondisi, import library) diperlukan sejak Pertemuan 12.

**Alat & Komponen Utama:**
ESP32 DevKit (min. 2 unit), ESP32-CAM + FTDI adapter, DHT22, LED, Relay, MPU6050 (IMU), Laptop dengan Python 3.x, MQTT Broker (lokal: Mosquitto / cloud: HiveMQ), Serial Monitor / MQTT Explorer.

---

### **Rancangan Perkuliahan (16 Pertemuan)**

---

### **BAGIAN 1: Komunikasi Antar ESP32 — Dari Kabel ke Nirkabel (Pertemuan 1–4)**

> Di kursus **IoT Dasar**, mahasiswa sudah mempelajari UART, I2C, dan SPI untuk menghubungkan ESP32 ke sensor/modul. Bagian ini memulai materi lanjutan dengan menggunakan **protokol yang sama** untuk konteks yang berbeda: **komunikasi antar-ESP32**. Kemudian berlanjut ke komunikasi nirkabel tanpa router (ESP-NOW).

---

#### **Pertemuan 1: Komunikasi Antar ESP32 — UART (ESP32 ↔ ESP32)**

- **Topik:** Menghubungkan dua ESP32 secara fisik via UART untuk pembagian tugas.
- **Konteks:**
  > Di IoT Dasar, UART digunakan untuk ESP32 ↔ Serial Monitor atau ESP32 ↔ modul GPS. Kini, protokol yang **sama** digunakan untuk komunikasi **langsung antara dua ESP32**.
- **Sub-Topik:**
  - **Mengapa komunikasi antar-ESP32?** Pembagian tugas: satu ESP32 khusus membaca sensor, ESP32 lain khusus mengontrol aktuator.
  - **UART ESP32-to-ESP32:** Cross-connect TX↔RX, GND bersama.
  - Protokol paket data sederhana: delimiter (`\n`), format `"KEY:VALUE"`, checksum sederhana.
  - Error handling: data corrupt, buffer overflow, timeout.
  - Perbandingan dengan IoT Dasar: ESP32→modul (satu arah) vs ESP32↔ESP32 (dua arah penuh).
- **Praktik:**
  1. **Sensor ESP32 (A):** Baca DHT22 → kirim `"T:28.5,H:65.2\n"` via UART TX.
  2. **Aktuator ESP32 (B):** Terima → parse data → nyalakan relay jika suhu > 30°C → kirim ACK `"OK\n"` kembali.
  3. Tambah LED indikator: LED blink setiap kali data diterima dengan benar.
  4. **Debugging:** ESP32-A juga mengirim log ke Serial Monitor via UART0 (multi-UART).
- **Komponen:** 2× ESP32 DevKit, DHT22, Relay, LED, Jumper wires.
- **Capaian Sesi:** Mahasiswa mampu membangun komunikasi dua arah antar-ESP32 menggunakan UART.

---

#### **Pertemuan 2: Komunikasi Antar ESP32 — I2C Master/Slave**

- **Topik:** Komunikasi antar ESP32 menggunakan bus I2C dalam mode Master-Slave.
- **Konteks:**
  > Di IoT Dasar, I2C digunakan untuk ESP32 (Master) → sensor/OLED (Slave). Kini, **ESP32 lain berfungsi sebagai Slave**, menjadikan dua mikrokontroler berkomunikasi dalam satu bus.
- **Sub-Topik:**
  - **I2C Slave Mode pada ESP32:** Library `Wire.h` → `Wire.onReceive()`, `Wire.onRequest()`.
  - Pengalamatan kustom: ESP32 Slave sebagai perangkat I2C dengan address `0x08`.
  - ESP32 Master mengirim perintah → ESP32 Slave merespons dengan data sensor.
  - Multi-Slave: menghubungkan ESP32 Slave + sensor OLED pada satu bus I2C yang sama.
  - Kapan memilih I2C vs UART untuk komunikasi antar-ESP32.
- **Praktik:**
  1. **ESP32 Slave (A):** Membaca DHT22, menyimpan suhu & kelembapan di variabel. Menunggu request dari Master.
  2. **ESP32 Master (B):** Minta data ke alamat I2C `0x08` setiap 3 detik → terima → tampilkan di OLED.
  3. Master mengirim perintah `0x01` (nyalakan) / `0x00` (matikan) → Slave menerima → kontrol LED.
  4. Gabungkan: OLED (I2C `0x3C`) + ESP32 Slave (`0x08`) dalam satu bus.
- **Komponen:** 2× ESP32 DevKit, DHT22, OLED 128×64, LED, Resistor pull-up 4.7kΩ.
- **Capaian Sesi:** Mahasiswa mampu menggunakan I2C Master/Slave untuk komunikasi antar-ESP32.

---

#### **Pertemuan 3: ESP-NOW — Komunikasi Nirkabel Point-to-Point**

- **Topik:** Komunikasi nirkabel langsung antar ESP32 tanpa router WiFi.
- **Sub-Topik:**
  - Apa itu ESP-NOW? Protokol proprietary Espressif. Range ±200m (LOS). Latensi rendah (<1ms).
  - Perbandingan ESP-NOW vs WiFi vs BLE: use case masing-masing.
  - Konsep: **MAC Address** sebagai pengalamatan, struktur **peer**, callback `OnDataSent` & `OnDataRecv`.
  - Cara mendapatkan MAC Address ESP32: `WiFi.macAddress()`.
  - Mengirim struct data (`typedef struct`) melalui ESP-NOW.
- **Praktik:**
  1. **Persiapan:** Catat MAC Address kedua ESP32.
  2. **Sender ESP32:** baca nilai DHT22 → kirim struct `{float suhu, float lembap, int id}` via ESP-NOW ke MAC Address Receiver.
  3. **Receiver ESP32:** terima data → tampilkan di OLED + Serial Monitor + nyalakan LED jika suhu > 35°C.
  4. Uji jangkauan: pindahkan ESP32 dan amati batas komunikasi.
- **Komponen:** 2× ESP32 DevKit, DHT22, OLED, LED.
- **Capaian Sesi:** Mahasiswa mampu membangun komunikasi nirkabel P2P menggunakan ESP-NOW.

---

#### **Pertemuan 4: ESP-NOW Gateway & Multi-Node Network**

- **Topik:** Membangun jaringan sensor nirkabel skala kecil dengan arsitektur Gateway.
- **Sub-Topik:**
  - Arsitektur **Star Topology**: banyak Node Sensor → satu Gateway ESP32.
  - Broadcast ESP-NOW: kirim ke semua peer sekaligus (`{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}`).
  - **Bridge ESP-NOW → MQTT**: Gateway menerima data dari semua node → forward ke MQTT Broker via WiFi.
  - Identifikasi node: gunakan ID atau MAC Address dalam payload.
  - Pertimbangan praktis: kapasitas antrian, error handling jika node tidak merespons.
- **Praktik:**
  1. **2 Node ESP32**: masing-masing mengirim data sensor (sensor berbeda, misal suhu & jarak) ke Gateway.
  2. **Gateway ESP32**: terima semua data → tampilkan di OLED → forward ke MQTT Broker sebagai topic `iot/node1/suhu`, `iot/node2/jarak`.
  3. Monitor semua topic di MQTT Explorer: amati data real-time dari kedua node.
- **Komponen:** 3× ESP32 (2 node + 1 gateway), DHT22, HC-SR04, OLED.
- **Capaian Sesi:** Mahasiswa mampu membangun jaringan multi-node ESP-NOW yang terhubung ke cloud via MQTT.

---

### **BAGIAN 2: Komunikasi Jaringan & REST API (Pertemuan 5–6)**

---

#### **Pertemuan 5: HTTP Client — GET Request & Konsumsi REST API**

- **Topik:** ESP32 sebagai konsumen layanan web (HTTP Client).
- **Sub-Topik:**
  - Review singkat HTTP: metode GET vs POST, status code, header, body.
  - Library `HTTPClient` pada ESP32: `http.begin()`, `http.GET()`, `http.getString()`.
  - Format data **JSON** dan cara parsing menggunakan library **ArduinoJson**.
  - Konsumsi API publik: contoh Open-Meteo (cuaca), ThingSpeak, atau JSONPlaceholder.
  - Error handling: timeout, kode HTTP 4xx/5xx, koneksi gagal.
- **Praktik:**
  1. ESP32 melakukan GET request ke `http://worldtimeapi.org/api/ip` → tampilkan waktu sekarang di Serial Monitor.
  2. ESP32 mengambil data cuaca dari Open-Meteo API → parse JSON → tampilkan suhu & kecepatan angin di OLED.
  3. Implementasi *retry logic*: jika request gagal, coba ulang 3x sebelum menyerah.
- **Tools:** Serial Monitor, browser (untuk test API manual), OLED 128×64.
- **Capaian Sesi:** Mahasiswa mampu mengonsumsi REST API publik dan mem-parsing JSON dari ESP32.

---

#### **Pertemuan 6: HTTP Client — POST Request & Pengiriman Data ke Server**

- **Topik:** Mengirim data sensor ESP32 ke server menggunakan HTTP POST.
- **Sub-Topik:**
  - HTTP POST: body request, `Content-Type: application/json`, header kustom.
  - Library `HTTPClient`: `http.POST(payload)`, `http.addHeader()`.
  - Backend sederhana: menggunakan **webhook.site** atau **Pipedream** untuk menangkap data.
  - Membangun payload JSON dari data sensor menggunakan ArduinoJson (`JsonDocument`).
  - Konsep API Key & autentikasi dasar (Bearer Token di header).
  - **ThingSpeak / Adafruit IO**: Mengirim data ke platform dengan HTTP POST.
- **Praktik:**
  1. ESP32 mengumpulkan data DHT22 → buat JSON payload → POST ke `webhook.site` → verifikasi di browser.
  2. Kirim data suhu + kelembapan ke **ThingSpeak** setiap 15 detik via HTTP POST → lihat grafik di ThingSpeak dashboard.
  3. Tambah header API Key pada request untuk autentikasi.
- **Tools:** webhook.site, ThingSpeak, DHT22.
- **Capaian Sesi:** Mahasiswa mampu mengirim data sensor terstruktur ke server via HTTP POST.

---

### **BAGIAN 3: Protokol MQTT — Pub/Sub Architecture (Pertemuan 7–8)**

---

#### **Pertemuan 7: Arsitektur MQTT & Setup Broker**

- **Topik:** Memahami standar pesan IoT berbasis *Publish/Subscribe*.
- **Sub-Topik:**
  - Mengapa MQTT? Keterbatasan HTTP untuk IoT (overhead, polling, always-request).
  - Arsitektur Pub/Sub: **Publisher** (ESP32 sensor), **Subscriber** (dashboard/app), **Broker** (Mosquitto).
  - Konsep: **Topic** (hierarki: `rumah/kamar/suhu`), **Payload** (data), **QoS** (0, 1, 2), **Retain**, **Last Will & Testament (LWT)**.
  - Instalasi MQTT Broker lokal: **Mosquitto** di PC/laptop.
  - Tools client MQTT: **MQTT Explorer** (GUI), `mosquitto_pub` / `mosquitto_sub` (CLI).
- **Praktik:**
  1. Instalasi Mosquitto di lokal. Start broker dengan konfigurasi default.
  2. Gunakan MQTT Explorer: publish pesan manual ke topic `test/hello` → subscribe dan terima di tab lain.
  3. Test QoS 0, 1, 2: amati perbedaan perilaku pengiriman.
  4. Uji Retained Message: publish retain → disconnect → reconnect → pesan masih ada.
- **Tools:** PC/Laptop, Mosquitto Broker, MQTT Explorer.
- **Capaian Sesi:** Mahasiswa memahami konsep Pub/Sub dan mampu menjalankan MQTT Broker lokal.

---

#### **Pertemuan 8: Implementasi MQTT pada ESP32**

- **Topik:** Mengintegrasikan ESP32 sebagai Publisher dan Subscriber MQTT.
- **Sub-Topik:**
  - Library **PubSubClient** (by Nick O'Leary): setup, `client.connect()`, `client.publish()`, `client.subscribe()`, callback.
  - Manajemen koneksi: *reconnect loop* – jika broker terputus, ESP32 otomatis connect ulang.
  - Publish data sensor berkala (non-blocking dengan `millis()`).
  - Subscribe dan parse perintah yang masuk (contoh: `{"relay":"ON"}`).
  - **Broker cloud:** Menggunakan **HiveMQ Cloud** (free tier) atau `broker.hivemq.com` (public) agar bisa diakses dari internet.
- **Praktik:**
  1. ESP32 **Publisher**: publish data suhu DHT22 ke topic `iot/kamar/suhu` setiap 5 detik.
  2. Monitor data di MQTT Explorer secara real-time.
  3. ESP32 **Subscriber**: subscribe ke topic `iot/kamar/relay` → jika payload `"ON"`, nyalakan relay; `"OFF"` matikan.
  4. Gabungkan: ESP32 sekaligus Publisher sensor **dan** Subscriber perintah relay.
- **Tools:** PubSubClient library, DHT22, Relay, HiveMQ Cloud, MQTT Explorer.
- **Capaian Sesi:** Mahasiswa mampu membangun sistem IoT full-duplex berbasis MQTT.

---

### **BAGIAN 4: Edge AI / TinyML (Pertemuan 9–10)**

---

#### **Pertemuan 9: Konsep Edge AI & TensorFlow Lite Micro**

- **Topik:** Menjalankan kecerdasan buatan langsung di mikrokontroler (Edge Inference).
- **Sub-Topik:**
  - **Edge vs Cloud AI:**

    | Aspek | Cloud AI | Edge AI |
    |---|---|---|
    | Latensi | Tinggi (round-trip) | Sangat rendah (lokal) |
    | Privasi | Data dikirim ke server | Data tetap di perangkat |
    | Koneksi | Wajib online | Bisa offline |
    | Daya komputasi | Tidak terbatas | Sangat terbatas |

  - Apa itu **TinyML**? Pipeline: Collect → Train → Convert (TFLite) → Deploy ke MCU.
  - **TensorFlow Lite Micro (TFLM)**: format model `.tflite`, quantisasi (float32 → int8).
  - Library Arduino: `EloquentTinyML`, `TensorFlowLiteESP32`.
  - Model pre-trained yang bisa dijalankan di ESP32: gesture classifier, anomaly detection, keyword spotting.
  - Keterbatasan hardware: RAM ±320KB, flash 4MB. Trade-off akurasi vs ukuran model.
- **Praktik:**
  1. Deploy model **"Hello ML" sederhana**: regresi sinusoidal (contoh resmi TFLite Micro) ke ESP32.
  2. Output prediksi = nilai LED brightness (demonstrasi bahwa ESP32 bisa menjalankan inferensi).
  3. Monitor output inferensi di Serial Monitor: bandingkan output model vs nilai ground truth.
- **Tools:** Arduino IDE, library `EloquentTinyML` atau `TensorFlowLiteESP32`.
- **Capaian Sesi:** Mahasiswa memahami konsep Edge AI dan mampu mendeploy model TFLite Micro ke ESP32.

---

#### **Pertemuan 10: TinyML — Anomaly Detection & Gesture Classification**

- **Topik:** Aplikasi praktis TinyML pada data sensor dan input IMU.
- **Sub-Topik:**
  - **Anomaly Detection pada data sensor:** Model sederhana mendeteksi perilaku tidak normal dari time-series sensor (suhu tiba-tiba spike).
  - **Gesture Classification dengan IMU (MPU6050):** Rekam data akselerometer untuk beberapa gesture → latih model sederhana dengan **Edge Impulse** → export ke `.tflite` → deploy ke ESP32.
  - Pengenalan **Edge Impulse Studio**: platform no-code untuk TinyML (data capture → processing → ML training → deploy).
  - Alternatif tanpa IMU: klasifikasi berdasarkan data time-series DHT22 (normal vs abnormal).
  - Integrasi hasil inferensi dengan aktuator: jika anomaly terdeteksi → kirim alert via MQTT.
- **Praktik:**
  1. Buat akun Edge Impulse, buat project baru "Gesture Classifier".
  2. Rekam data dari MPU6050 untuk 2 gesture (contoh: shake vs still) via Serial data forwarder.
  3. Latih model di Edge Impulse (Neural Network) → download library Arduino `.zip`.
  4. Deploy ke ESP32: inferensi real-time → LED merah jika gesture "shake", LED hijau jika "still".
  5. Tambah MQTT alert: jika "shake" terdeteksi → publish ke topic `iot/gesture/alert`.
- **Komponen:** ESP32, MPU6050 (IMU), LED merah & hijau.
- **Capaian Sesi:** Mahasiswa mampu melatih dan mendeploy model TinyML untuk klasifikasi gesture real-time.

---

### **BAGIAN 5: Computer Vision — ESP32-CAM (Pertemuan 11–14)**

---

#### **Pertemuan 11: ESP32-CAM — Setup, Konfigurasi & Video Streaming**

- **Topik:** Memperkenalkan modul kamera ESP32-CAM dan membuat live streaming.
- **Sub-Topik:**
  - Pengenalan **ESP32-CAM**: chip AI Thinker, sensor kamera OV2640, slot microSD, flash LED.
  - Upload program: perlu **FTDI/CP2102 adapter** (GPIO0 → GND saat upload).
  - Library: `esp_camera.h` (built-in ESP32 core). Konfigurasi pin untuk board AI Thinker.
  - **CameraWebServer** (contoh bawaan Arduino IDE): streaming MJPEG via HTTP.
  - Parameter kamera: resolusi (QQVGA → UXGA), frame rate, kualitas JPEG, exposure, white balance.
  - Akses stream: `http://IP_ESP32_CAM/stream` dari browser di jaringan yang sama.
- **Praktik:**
  1. Pasang FTDI programmer → upload CameraWebServer → lepas GPIO0 dari GND → reset.
  2. Buka `http://[IP]/` di browser → akses control panel → lihat live stream.
  3. Eksperimen resolusi: bandingkan QVGA vs VGA vs SVGA dalam hal frame rate & kualitas.
  4. Akses endpoint stream MJPEG (`/stream`) dari Python script sederhana dengan `urllib`.
- **Komponen:** ESP32-CAM AI Thinker, FTDI Adapter, Kabel USB.
- **Capaian Sesi:** Mahasiswa mampu mengoperasikan ESP32-CAM dan mengakses live stream dari browser/Python.

---

#### **Pertemuan 12: Computer Vision On-Device — Deteksi & Pengenalan Wajah**

- **Topik:** Pemrosesan citra langsung di ESP32-CAM tanpa komputer.
- **Sub-Topik:**
  - Library **ESP-WHO** (Espressif): face detection + face recognition di ESP32.
  - Algoritma deteksi wajah ringan yang bisa berjalan di MCU: MTMN (MobileNet-based).
  - Proses **face enrollment**: mendaftarkan wajah yang dikenali ke flash memory.
  - Trigger output: jika wajah **dikenal** → buka kunci (servo/relay). Jika tidak dikenal → alarm.
  - Keterbatasan: resolusi terbatas, latensi ~500ms per frame, tidak ideal untuk banyak wajah.
- **Praktik:**
  1. Flash firmware ESP-WHO `face_recognition` ke ESP32-CAM.
  2. Buka halaman web streaming → aktifkan "Face Detection": amati bounding box di sekitar wajah.
  3. Enroll 1 wajah via web interface → aktifkan "Face Recognition".
  4. Jika wajah dikenal → LED hijau menyala + relay trigger (simulasi kunci pintu).
  5. Jika wajah tidak dikenal → LED merah + buzzer.
- **Komponen:** ESP32-CAM, LED hijau & merah, Relay, Servo (opsional), Buzzer.
- **Capaian Sesi:** Mahasiswa mampu mengimplementasikan sistem akses berbasis pengenalan wajah on-device.

---

#### **Pertemuan 13: Computer Vision dengan Python & OpenCV — Color Tracking**

- **Topik:** Memproses video ESP32-CAM menggunakan Python di komputer.
- **Sub-Topik:**
  - Mengapa pindah ke komputer? Komputer memiliki daya komputasi jauh lebih besar untuk CV kompleks.
  - Membaca video stream dari ESP32-CAM di Python: `cv2.VideoCapture("http://IP/stream")`.
  - Konsep citra digital: piksel, kanal warna RGB & HSV.
  - **HSV Color Space**: lebih robust untuk deteksi warna dibanding RGB. `cv2.cvtColor()`.
  - **Color Masking**: `cv2.inRange()` untuk isolasi warna tertentu.
  - Morphological operations: erode, dilate untuk membersihkan noise.
  - Deteksi kontur: `cv2.findContours()`, centroid objek.
  - Mengirim koordinat tracking kembali ke ESP32 via MQTT (feedback loop).
- **Praktik:**
  1. Python script: baca stream ESP32-CAM → tampilkan frame dengan OpenCV.
  2. Color Tracking: deteksi bola merah (HSV mask) → gambar bounding box di sekitarnya.
  3. Hitung centroid bola → publish koordinat `{x, y}` ke MQTT topic `iot/tracking`.
  4. ESP32 (separate) subscribe ke topic tersebut → gerakkan servo ke arah bola.
- **Tools:** Python 3, OpenCV (`pip install opencv-python`), ESP32-CAM, Motor Servo.
- **Capaian Sesi:** Mahasiswa mampu membangun sistem color tracking berbasis ESP32-CAM + Python.

---

#### **Pertemuan 14: Object Detection dengan YOLO & Gesture Recognition**

- **Topik:** Deteksi objek dan pengenalan gesture untuk kontrol IoT.
- **Sub-Topik:**
  - **YOLO (You Only Look Once):** Algoritma single-pass detection, bounding box, confidence score, class label.
  - Versi YOLO: YOLOv5, YOLOv8 (Ultralytics), YOLOv11. Pilih **YOLOv8n** (nano) untuk hardware rendah.
  - Pre-trained model COCO: 80 class objek (orang, mobil, motor, HP, laptop, dll).
  - **Ultralytics API**: `model = YOLO("yolov8n.pt")`, `results = model(frame)`.
  - **MediaPipe (Google):** Framework open-source untuk hand tracking, 21 landmark per tangan.
  - Klasifikasi gesture: OPEN (semua jari lurus) vs FIST (mengepal) vs POINT (telunjuk saja).
  - Integrasi YOLO & MediaPipe dengan ESP32 via MQTT: deteksi/gesture → perintah → aktuator.
- **Praktik:**
  1. `pip install ultralytics` → jalankan YOLO pada stream ESP32-CAM: deteksi objek, gambar bounding box.
  2. Filter class "person": jika terdeteksi → publish MQTT alert `{"event":"person_detected","count":2}`.
  3. `pip install mediapipe` → hand tracking: klasifikasi 3 gesture → publish via MQTT.
  4. **Integrasi penuh:** Video dari ESP32-CAM → YOLO/MediaPipe di Python → MQTT → ESP32 respons aktuator.
- **Tools:** Python 3, Ultralytics, MediaPipe, ESP32-CAM, ESP32 (aktuator), LED, Buzzer.
- **Capaian Sesi:** Mahasiswa mampu membangun sistem deteksi objek dan kontrol gesture berbasis computer vision.

---

### **BAGIAN 6: Persiapan & Proyek Akhir (Pertemuan 15–16)**

---

#### **Pertemuan 15: Arsitektur Sistem Terpadu & Persiapan Proyek Akhir**

- **Topik:** Menggabungkan semua elemen kursus dan mempersiapkan proyek akhir.
- **Sub-Topik:**
  - **Arsitektur sistem IoT terpadu:**
    ```
    [Sensor Node (ESP-NOW)] ──► [Gateway ESP32] ──► [MQTT Broker]
                                                          │
    [ESP32-CAM] ──► [Python CV Server] ──► [MQTT Broker] ◄┘
                                                          │
                                              [Subscriber ESP32]
                                                  (Aktuator)
                                                          │
                                              [Dashboard / Alert]
    ```
  - Desain system: pemilihan protokol yang tepat untuk setiap komponen.
  - *Best practices*: pemisahan concern, kode modular, error handling, reconnect otomatis.
  - Manajemen daya: deep sleep ESP32, optimisasi konsumsi arus untuk battery-powered node.
  - Briefing proyek akhir: persyaratan, rubrik penilaian, format demo.
  - Konsultasi desain proyek kelompok/individu dengan dosen.
- **Workshop:**
  1. Setiap kelompok menggambar arsitektur sistem proyek mereka (diagram blok).
  2. Validasi pilihan komponen, protokol, dan tools yang akan digunakan.
  3. Debugging dan penyelesaian masalah teknis yang dihadapi selama kursus.
- **Capaian Sesi:** Setiap mahasiswa/kelompok memiliki desain proyek final yang disetujui.

---

#### **Pertemuan 16: Proyek Akhir — Presentasi & Demo Sistem**

- **Topik:** Demonstrasi proyek IoT terpadu yang menggabungkan komunikasi data, computer vision, dan/atau Edge AI.

- **Spesifikasi Proyek (Minimal):**
  - Menggunakan minimal **2 protokol komunikasi** (contoh: ESP-NOW + MQTT, atau HTTP + MQTT).
  - Menggunakan minimal **2 sensor** berbeda.
  - Memiliki komponen **Computer Vision** atau **Edge AI/TinyML**.
  - Sistem dapat dikendalikan atau dimonitor secara **remote**.
  - Kode bersih, modular, dan terdokumentasi.

- **Contoh Ide Proyek:**

  | Proyek | Teknologi | Kompleksitas |
  |---|---|---|
  | **Smart Security System** | ESP32-CAM + YOLO (person detect) → MQTT alert → buzzer ESP32 + Blynk notifikasi | ⭐⭐⭐ |
  | **Gesture-Controlled Smart Home** | ESP32-CAM + MediaPipe → MQTT → relay (lampu/AC/kipas) | ⭐⭐⭐ |
  | **Distributed Environment Monitor** | 3 Node ESP-NOW → Gateway → MQTT → Dashboard web | ⭐⭐⭐ |
  | **Smart Attendance System** | ESP32-CAM face recognition + MQTT log + Web dashboard | ⭐⭐⭐⭐ |
  | **Anomaly Detection Pipeline** | Sensor ESP32 → TinyML anomaly detect → MQTT alert → aktuator | ⭐⭐⭐⭐ |
  | **Gesture Pan-Tilt Camera** | MediaPipe hand tracking → servo pan-tilt via MQTT | ⭐⭐ |
  | **Smart Parking Lot** | HC-SR04 multi-node + ESP-NOW gateway + YOLO counting + MQTT dashboard | ⭐⭐⭐⭐⭐ |

- **Format Presentasi (15 menit/kelompok):**
  1. **Demo alat (5 menit):** Demonstrasikan semua fitur sistem berjalan secara langsung.
  2. **Arsitektur sistem (3 menit):** Jelaskan diagram blok dan aliran data.
  3. **Highlight kode (3 menit):** Tunjukkan bagian kode paling kritis/inovatif.
  4. **Tanya jawab (4 menit):** Dosen & mahasiswa lain memberikan pertanyaan.

- **Rubrik Penilaian:**

  | Kriteria | Bobot |
  |---|---|
  | Fungsionalitas sistem (semua fitur berjalan) | 35% |
  | Kompleksitas & integrasi teknologi | 25% |
  | Kualitas kode (modular, terdokumentasi, error handling) | 20% |
  | Presentasi & kemampuan teknis menjelaskan | 20% |

---

### **Referensi & Sumber Belajar**

- **ESP32 & MQTT:** [Random Nerd Tutorials](https://randomnerdtutorials.com/projects-esp32/), [PubSubClient Docs](https://pubsubclient.knolleary.net/)
- **ESP-NOW:** [Espressif ESP-NOW Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_now.html)
- **TinyML / Edge Impulse:** [Edge Impulse Documentation](https://docs.edgeimpulse.com/), [TensorFlow Lite Micro](https://www.tensorflow.org/lite/microcontrollers)
- **OpenCV Python:** [OpenCV Documentation](https://docs.opencv.org/4.x/), [PyImageSearch](https://pyimagesearch.com/)
- **YOLO:** [Ultralytics YOLOv8 Docs](https://docs.ultralytics.com/)
- **MediaPipe:** [MediaPipe Solutions](https://developers.google.com/mediapipe/solutions)
- **MQTT:** [HiveMQ MQTT Essentials](https://www.hivemq.com/mqtt-essentials/), [MQTT Explorer](https://mqtt-explorer.com/)
