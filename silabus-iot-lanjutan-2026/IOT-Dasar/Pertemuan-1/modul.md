# Modul Pertemuan 1 — IOT Dasar
# Pengenalan Internet of Things & Ekosistem ESP32

---

## 1. Maksud dan Tujuan Materi

### Maksud
Pertemuan pertama dirancang sebagai fondasi konseptual sebelum mahasiswa menyentuh perangkat keras. Memahami **apa itu IoT**, bagaimana arsitekturnya bekerja, dan mengapa **ESP32** menjadi platform dominan di industri akan membentuk pola pikir yang benar untuk seluruh perkuliahan.

### Tujuan Pembelajaran
Setelah mengikuti pertemuan ini, mahasiswa mampu:
1. **Mendefinisikan** Internet of Things (IoT) dan membedakannya dari jaringan komputer konvensional.
2. **Menjelaskan** empat lapisan arsitektur IoT: *Things → Gateway → Cloud → Application*.
3. **Mengidentifikasi** contoh nyata implementasi IoT di minimal 4 domain industri.
4. **Mendeskripsikan** spesifikasi teknis ESP32 sebagai System on a Chip (SoC).
5. **Membedakan** varian modul dan papan pengembang ESP32.
6. **Mengenali** ekosistem tools pengembangan: PlatformIO, Arduino IDE, ESP-IDF, simulator Wokwi.

---

## 2. Teori Materi

### 2.1 Apa itu Internet of Things (IoT)?

**Internet of Things** adalah perluasan konektivitas internet kepada objek-objek fisik — bukan hanya komputer dan smartphone, tetapi juga lampu rumah, termostat, mesin pabrik, sensor pertanian, hingga kendaraan. Setiap "thing" dilengkapi sensor, aktuator, dan kemampuan komunikasi sehingga dapat mengumpulkan data, mengirimkannya, dan merespons secara otomatis.

> **Definisi ITU-T:** *"IoT adalah infrastruktur global untuk masyarakat informasi, yang memungkinkan layanan-layanan canggih dengan menghubungkan berbagai hal (fisik maupun virtual) berdasarkan teknologi informasi dan komunikasi yang ada dan yang terus berkembang."*

**Skala Global IoT:**
- Tahun 2024: lebih dari **17 miliar** perangkat IoT aktif di seluruh dunia
- Proyeksi 2030: **30+ miliar** perangkat
- Nilai pasar global IoT: diproyeksikan **USD 1 triliun** pada 2030

**IoT vs Jaringan Komputer Tradisional:**

| Aspek | Jaringan Tradisional | IoT |
|---|---|---|
| Node | Server, PC, smartphone | Sensor, aktuator, mikrokontroler |
| Daya | Catu daya stabil (listrik PLN) | Sering baterai / solar panel |
| Konektivitas | Ethernet, WiFi stabil | WiFi, BLE, LoRa, Zigbee, ESP-NOW |
| Data | Request/response (HTTP) | Streaming kontinu, event-driven |
| Skala | Ratusan-ribuan | Jutaan-miliaran perangkat |
| Latensi | ms–detik | Kadang µs (real-time control) |

---

### 2.2 Arsitektur Umum IoT — Empat Lapisan

```
┌──────────────────────────────────────────────────────────────┐
│  LAPISAN 4: APPLICATION LAYER (Aplikasi Pengguna)            │
│  Dashboard Web, Mobile App, Sistem Otomasi, Notifikasi       │
│  Contoh: Grafana, Home Assistant, Blynk, Custom Web App      │
└────────────────────────────┬─────────────────────────────────┘
                             │  (REST API, WebSocket, MQTT)
┌────────────────────────────▼─────────────────────────────────┐
│  LAPISAN 3: CLOUD / PLATFORM LAYER                            │
│  Database, Analitik, Broker Pesan, AI/ML Processing           │
│  Contoh: AWS IoT, Google Cloud IoT, InfluxDB, Mosquitto       │
└────────────────────────────┬─────────────────────────────────┘
                             │  (WiFi, 4G/LTE, Ethernet)
┌────────────────────────────▼─────────────────────────────────┐
│  LAPISAN 2: GATEWAY / EDGE LAYER                              │
│  Agregasi Data, Pre-processing, Protocol Bridging             │
│  Contoh: Raspberry Pi, ESP32 Gateway, Edge Server             │
└────────────────────────────┬─────────────────────────────────┘
                             │  (Bluetooth, Zigbee, ESP-NOW, LoRa, I2C, SPI)
┌────────────────────────────▼─────────────────────────────────┐
│  LAPISAN 1: THINGS / PERCEPTION LAYER                         │
│  Sensor (DHT22, HC-SR04, LDR, BMP280)                        │
│  Aktuator (Servo, Relay, Motor DC, Buzzer)                    │
│  Mikrokontroler: ESP32, Arduino, STM32                        │
└──────────────────────────────────────────────────────────────┘
```

**Penjelasan Setiap Lapisan:**

**Lapisan 1 – Things (Perception Layer)**
Ujung tombak IoT — dunia fisik yang dapat dirasakan dan digerakkan.
- **Sensor** mengubah besaran fisik (suhu, cahaya, jarak, tekanan) menjadi sinyal listrik.
- **Aktuator** menerima perintah dan menggerakkan dunia fisik (motor putar, relay buka/tutup, buzzer).
- **Mikrokontroler (MCU)** seperti ESP32 berfungsi sebagai "otak" lokal yang membaca sensor, mengambil keputusan, dan berkomunikasi ke lapisan atas.

**Lapisan 2 – Gateway / Edge Layer**
Menjembatani protokol jarak dekat (Bluetooth, Zigbee, ESP-NOW) dengan protokol internet (WiFi, Ethernet). ESP32 sendiri sering berperan ganda: sebagai node sensor sekaligus gateway. **Edge Computing** menempatkan pemrosesan di sini untuk mengurangi latensi dan beban bandwidth ke cloud.

**Lapisan 3 – Cloud / Platform Layer**
Pusat data dan kecerdasan sistem:
- Penyimpanan data historis (InfluxDB, TimescaleDB)
- Analitik dan dashboarding (Grafana)
- Broker pesan untuk Pub/Sub (Mosquitto MQTT Broker)
- Device Management — OTA update, monitoring status perangkat

**Lapisan 4 – Application Layer**
Antarmuka yang dihadapi end-user:
- Dashboard monitoring (Grafana, Home Assistant)
- Aplikasi mobile (Blynk, HA Companion App)
- Sistem otomasi (Home Assistant Automations, Node-RED)

---

### 2.3 Studi Kasus Implementasi IoT

**A. Smart Home (Rumah Pintar)**

| Komponen | Fungsi |
|---|---|
| Sensor gerak PIR + ESP32-CAM | Deteksi gerakan → rekam keamanan |
| DHT22 + AC (relay) | Jaga suhu ruangan otomatis |
| LDR + relay lampu | Lampu otomatis saat gelap |
| Smart lock + fingerprint | Akses tanpa kunci fisik |
| Hub: Home Assistant (RPi) | Kontrol & automasi seluruh perangkat |
| App: HA Companion / Blynk | Kontrol dari smartphone di mana saja |

**B. Smart Agriculture (Pertanian Cerdas)**
- **Node Sensor:** Kelembaban tanah (kapasitif), suhu udara (DHT22), cahaya (BH1750)
- **Komunikasi:** ESP-NOW antar node → Gateway → MQTT → Dashboard
- **Aktuasi:** Pompa irigasi otomatis saat kelembaban tanah rendah
- **Hasil:** Penghematan air 30–40% vs irigasi manual

**C. Industry 4.0 / Smart Factory**
- Sensor getaran (accelerometer) pada mesin → deteksi keausan sebelum rusak (Predictive Maintenance)
- Sensor arus listrik → monitoring konsumsi daya real-time
- Integrasi MQTT ke SCADA/MES

**D. Smart City**
- Tempat sampah + sensor ultrasonik → truk hanya mengunjungi yang penuh
- Sensor kualitas udara (PM2.5) di banyak titik → dashboard publik
- Smart parking: sensor di setiap slot → aplikasi mengarahkan ke slot kosong

---

### 2.4 ESP32 — System on a Chip (SoC)

**System on a Chip (SoC)** adalah chip terintegrasi yang memuat CPU, memori, antarmuka I/O, dan modem komunikasi dalam satu keping silikon. ESP32 adalah SoC revolusioner untuk IoT karena menggabungkan semua kebutuhan dasar (WiFi + Bluetooth + GPIO + ADC) dalam satu chip berbiaya sangat rendah (~Rp 60K untuk development board).

### 2.5 Spesifikasi Teknis ESP32

| Parameter | Spesifikasi |
|---|---|
| **Chip** | Espressif ESP32-D0WD (Dual-Core) |
| **CPU** | Tensilica Xtensa LX6, 32-bit, Dual-Core |
| **Clock** | Hingga 240 MHz (konfigurasi: 80/160/240 MHz) |
| **SRAM** | 520 KB |
| **ROM** | 448 KB (bootloader + library dasar) |
| **Flash Eksternal** | 4 MB – 16 MB (pada modul WROOM-32) |
| **WiFi** | 802.11 b/g/n (2.4 GHz), hingga 150 Mbps |
| **Bluetooth** | v4.2 BR/EDR & BLE |
| **GPIO** | 34 pin GPIO yang dapat dikonfigurasi |
| **ADC** | 2 × SAR ADC 12-bit (18 channel) |
| **DAC** | 2 × DAC 8-bit |
| **UART** | 3 port |
| **SPI** | 4 bus (2 dapat digunakan bebas) |
| **I2C** | 2 bus |
| **I2S** | 2 bus (untuk audio) |
| **PWM** | 16 channel LEDC, resolusi hingga 16-bit |
| **Tegangan Kerja** | 3.3V (I/O toleran 3.3V, **BUKAN 5V!**) |
| **Konsumsi Daya** | Aktif WiFi: ~240 mA; Deep Sleep: ~10 µA |
| **RTC** | Real-Time Clock dengan Ultra-Low-Power co-processor |
| **Enkripsi** | Hardware AES, SHA2, RSA, ECC |
| **Suhu Operasi** | -40°C hingga +85°C |

> ⚠️ **PERINGATAN KRITIS:** Pin GPIO ESP32 beroperasi pada **3.3 Volt**. Jangan menghubungkan sinyal 5V langsung ke pin GPIO tanpa level shifter — ini **merusak chip secara permanen**!

---

### 2.6 Perbandingan ESP32 dengan Platform Lain

| Fitur | Arduino Uno | ESP8266 | **ESP32** | Raspberry Pi 4 |
|---|:---:|:---:|:---:|:---:|
| **Harga** | ~Rp 50K | ~Rp 35K | **~Rp 60K** | ~Rp 900K |
| **CPU** | 8-bit, 16 MHz | 32-bit, 80 MHz | **32-bit, 240 MHz** | 64-bit, 1.5 GHz |
| **Core** | Single | Single | **Dual** | Quad |
| **RAM** | 2 KB | 80 KB | **520 KB** | 4/8 GB |
| **WiFi** | ❌ | ✅ 2.4 GHz | **✅ 2.4 GHz** | ✅ 2.4/5 GHz |
| **Bluetooth** | ❌ | ❌ | **✅ BT 4.2 + BLE** | ✅ BT 5.0 |
| **GPIO** | 14 | 17 | **34** | 26 |
| **ADC** | 10-bit | 10-bit (1 ch) | **12-bit (18 ch)** | Via I2C ADC |
| **OS** | Bare Metal | Bare Metal | **FreeRTOS** | Linux |
| **Deep Sleep** | ✅ ~25 µA | ✅ ~20 µA | **✅ ~10 µA** | ❌ |

---

### 2.7 Varian Board & Modul ESP32

**A. Modul Inti**

| Modul | Chip | Fitur | Penggunaan |
|---|---|---|---|
| **ESP32-WROOM-32D** | ESP32-D0WD | Antena PCB | Produksi massal |
| **ESP32-WROOM-32U** | ESP32-D0WD | Antena IPEX (external) | Jangkauan luas |
| **ESP32-S3-WROOM** | ESP32-S3 | USB Native, AI Acceleration | ML on-device |
| **ESP32-C3-MINI** | ESP32-C3 (RISC-V) | BLE 5.0, footprint kecil | Produk compact |

**B. Development Board**

| Board | Chip | Fitur Khusus | Ideal Untuk |
|---|---|---|---|
| **ESP32 DevKit v1** | WROOM-32 | 38 pin, USB-Serial CP2102 | **Pembelajaran ✅** |
| **ESP32-CAM** | ESP32 + OV2640 | Kamera 2MP, SD Card | Computer Vision |
| **ESP32-S3-DevKitC** | ESP32-S3 | 2× USB-C, PSRAM 8MB | TinyML, Display |
| **ESP32-C3-DevKitM** | ESP32-C3 | USB-C, harga murah | Proyek sederhana |

---

### 2.8 Ekosistem Tools Pengembangan

**A. PlatformIO (Utama dalam kursus ini)**
- IDE profesional berbasis Visual Studio Code
- Manajemen library otomatis (didefinisikan di `platformio.ini`)
- Multi-board support (ESP32, Arduino, STM32, dll.)
- Build system yang powerful (CMake-based)
- **Rekomendasi untuk semua pertemuan di kursus ini**

**B. Arduino IDE (Alternatif)**
- Lebih sederhana, cocok untuk pemula absolut
- Library Manager dengan ribuan library
- Board Manager untuk menginstal dukungan ESP32

**C. ESP-IDF (Espressif IoT Development Framework)**
- Framework native dari Espressif, berbasis C
- Akses penuh ke seluruh fitur hardware
- Lebih kompleks, untuk engineer profesional

**D. ESPHome (Dibahas di Pertemuan 12-13)**
- Framework deklaratif berbasis YAML — tidak perlu menulis kode C++
- Terintegrasi langsung dengan Home Assistant
- Ideal untuk smart home dan automasi

**E. Simulator Wokwi**
- Simulator online gratis di [wokwi.com](https://wokwi.com)
- Mendukung ESP32, Arduino, dan banyak sensor/aktuator virtual
- Berguna untuk latihan tanpa hardware fisik

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Observasi Hardware ESP32

**Tujuan:** Mengenali bagian-bagian fisik papan ESP32 DevKit secara langsung.

**Bahan:** 1× ESP32 DevKit v1

**Langkah:**
1. Pegang papan ESP32 DevKit dan identifikasi komponen berikut:

```
            ESP32 DevKit v1 — Tampak Atas
    ┌─────────────────────────────────────────┐
    │  [USB Micro-B / USB-C]                  │
    │  ┌─────────────┐                        │
    │  │  CP2102 /    │ ← Chip USB-to-UART    │
    │  │  CH340       │   (jembatan USB↔Serial)│
    │  └─────────────┘                        │
    │                                         │
    │  [BOOT]  [EN/RST]  ← Tombol             │
    │                                         │
    │  ┌─────────────────────────────┐        │
    │  │                             │        │
    │  │    ESP32-WROOM-32 Module    │        │
    │  │    (kotak logam perak)      │        │
    │  │                             │        │
    │  │  ┌───────────────────┐     │        │
    │  │  │  Antena WiFi/BT   │     │ ← PCB  │
    │  │  │  (area tanpa      │     │   trace │
    │  │  │   komponen)       │     │        │
    │  │  └───────────────────┘     │        │
    │  └─────────────────────────────┘        │
    │                                         │
    │  Pin ● ● ● ● ● ● ● ● ● ● ● ● ● ● ●  │
    │  GND 3V3 ... GPIO ...                   │
    │  Pin ● ● ● ● ● ● ● ● ● ● ● ● ● ● ●  │
    └─────────────────────────────────────────┘
```

2. Identifikasi dan beri label:
   - **Modul ESP32-WROOM-32:** Kotak logam perak di tengah (berisi chip ESP32 + crystal + memori flash)
   - **Chip USB-to-UART:** IC berlabel `CP2102` atau `CH340` dekat port USB
   - **Tombol EN/RST:** Me-restart ESP32
   - **Tombol BOOT:** Jika ditahan saat power-on → mode upload firmware
   - **LED Built-in:** LED biru kecil, terhubung ke GPIO 2
   - **Pin Header:** Deretan pin kiri-kanan = antarmuka GPIO
   - **Antena:** Area PCB tanpa komponen di ujung modul

3. Buat sketsa papan ESP32 dan beri label setiap komponen.

---

### Praktikum 3.2 — Eksplorasi Proyek IoT Online

**Tujuan:** Mendapatkan inspirasi tentang kemungkinan yang bisa dibangun dengan ESP32.

**Langkah:**
1. Buka browser, kunjungi:
   - [Hackster.io](https://www.hackster.io) → Cari "ESP32"
   - [RandomNerdTutorials.com](https://randomnerdtutorials.com) → Kategori ESP32
   - [Instructables.com](https://www.instructables.com) → Cari "ESP32 IoT"

2. Pilih **satu proyek** yang menarik. Catat:
   - Nama proyek dan masalah yang dipecahkan
   - Sensor yang digunakan
   - Aktuator yang digunakan
   - Cara data dikirim ke cloud/dashboard

3. Presentasikan temuan ke kelas (2 menit).

---

### Praktikum 3.3 — Eksplorasi Simulator Wokwi (Opsional)

**Langkah:**
1. Buka [https://wokwi.com](https://wokwi.com)
2. Klik **"New Project"** → pilih **"ESP32"**
3. Program default: Blink. Klik **▶ Play** untuk mensimulasikan.
4. Ubah `delay(500)` → `delay(100)`, jalankan ulang — perhatikan perbedaan kecepatan kedipan.

---

## 4. Referensi

### Dokumentasi Resmi
1. **Espressif.** *ESP32 Technical Reference Manual.* [https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
2. **Espressif.** *ESP32 Datasheet.* [https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)

### Tutorial & Komunitas
3. **Random Nerd Tutorials.** *ESP32 Projects.* [https://randomnerdtutorials.com/projects-esp32/](https://randomnerdtutorials.com/projects-esp32/)
4. **Hackster.io.** *ESP32 Community.* [https://www.hackster.io/esp](https://www.hackster.io/esp)

### Tools
5. **PlatformIO.** [https://platformio.org/](https://platformio.org/)
6. **Wokwi Simulator.** [https://wokwi.com](https://wokwi.com)
7. **Home Assistant.** [https://www.home-assistant.io/](https://www.home-assistant.io/)
8. **ESPHome.** [https://esphome.io/](https://esphome.io/)
