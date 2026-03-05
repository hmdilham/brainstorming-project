# Materi IOT Dasar

---

### **Silabus Mata Kuliah: Dasar-Dasar IoT dengan ESP32**

**Deskripsi Mata Kuliah:**
Mata kuliah ini memberikan pengenalan komprehensif mengenai pengembangan perangkat Internet of Things (IoT) menggunakan board mikrokontroler ESP32. Mahasiswa akan mempelajari konsep dasar, arsitektur, dan perbandingan ESP32 dengan platform lain. Fokus utama adalah pada praktik langsung, mulai dari instalasi lingkungan pengembangan (Arduino IDE dan PlatformIO), pemahaman protokol komunikasi (UART, I2C, SPI), interaksi dengan berbagai sensor dan aktuator, komunikasi nirkabel (WiFi & Bluetooth), penggunaan library populer, pembuatan Web Server lokal, hingga integrasi dengan platform IoT (Blynk/MQTT) untuk membangun proyek nyata.

**Capaian Pembelajaran:**
- Mampu menyiapkan dan mengkonfigurasi lingkungan pengembangan untuk ESP32 (Arduino IDE & PlatformIO).
- Memahami perbedaan fundamental dan keunggulan ESP32 dibandingkan Arduino Uno.
- Menguasai implementasi protokol komunikasi serial (UART), I2C, dan SPI.
- Mampu membaca data dari berbagai jenis sensor (digital, analog, jarak, suhu).
- Mampu mengendalikan berbagai jenis aktuator (servo, relay, buzzer, LED RGB).
- Mampu menghubungkan ESP32 ke jaringan WiFi dan melakukan komunikasi Bluetooth BLE.
- Mampu menggunakan library populer seperti WiFiManager, ArduinoJson, dan NTPClient.
- Mampu membangun Web Server lokal berbasis ESP32 untuk monitoring dan kontrol.
- Mampu mengintegrasikan ESP32 dengan platform IoT untuk monitoring dan kontrol jarak jauh.
- Mampu merancang dan mengimplementasikan proyek IoT sederhana secara mandiri.

**Prasyarat:** Tidak ada prasyarat khusus. Dasar logika pemrograman (if, loop, variabel) akan sangat membantu.

**Alat & Komponen Utama:**
ESP32 DevKit, LED, Resistor, Push Button, Potensiometer, DHT11/DHT22, HC-SR04, PIR, LDR, Motor Servo, Relay Module, Buzzer, OLED 0.96" (I2C), Modul SD Card (SPI), Breadboard, Kabel Jumper.

---

### **Rancangan Perkuliahan (16 Pertemuan)**

---

#### **Pertemuan 1: Pengenalan IoT & Ekosistem ESP32**

- **Topik:** Konsep dasar Internet of Things dan pengenalan platform ESP32.
- **Sub-Topik:**
  - Apa itu IoT? Arsitektur umum IoT: *Things → Gateway → Cloud → Application*.
  - Studi kasus nyata: Smart Home, Smart Agriculture, Smart Factory, Smart City.
  - Pengenalan *System on a Chip* (SoC) ESP32: CPU Xtensa dual-core, WiFi 802.11 b/g/n, Bluetooth 4.2 BLE, 34 GPIO.
  - Perbedaan varian board ESP32: ESP32-WROOM-32, ESP32-S3, ESP32-C3, DevKitC.
  - Overview tools yang akan digunakan: Arduino IDE, PlatformIO, Fritzing.
- **Praktik:** Eksplorasi berbagai proyek ESP32 di platform (Hackster.io, Instructables). Diskusi kelompok: ide proyek IoT masing-masing mahasiswa.
- **Komponen:** -
- **Capaian Sesi:** Mahasiswa memahami ekosistem IoT dan posisi ESP32 di dalamnya.

---

#### **Pertemuan 2: Instalasi IDE & Proyek Pertama**

- **Topik:** Menyiapkan Lingkungan Pengembangan.
- **Sub-Topik:**
  - **Arduino IDE:** Instalasi Arduino IDE 2.x, tambah URL board ESP32 (Espressif), instalasi driver USB-UART (CH340 / CP210x).
  - **PlatformIO (Alternatif):** Instalasi VS Code + ekstensi PlatformIO, buat proyek baru.
  - Struktur program Arduino: `setup()`, `loop()`, komentar, tipe data dasar (`int`, `float`, `bool`, `String`).
  - Proses kompilasi dan upload ke board (USB port, baud rate Serial Monitor).
- **Praktik:**
  1. Upload program **Blink** (LED internal berkedip setiap 500ms).
  2. Modifikasi: LED berkedip 3x cepat, berhenti 1 detik, ulangi.
- **Komponen:** ESP32 DevKit, Kabel USB.
- **Capaian Sesi:** Mahasiswa berhasil mengupload program pertama ke ESP32.

---

#### **Pertemuan 3: GPIO Output — Digital & PWM**

- **Topik:** Mengendalikan Output Digital dan Analog (PWM).
- **Sub-Topik:**
  - Konsep pin GPIO: `INPUT`, `OUTPUT`, `INPUT_PULLUP`.
  - Fungsi `digitalWrite()`, `digitalRead()`, `pinMode()`.
  - Konsep PWM (*Pulse Width Modulation*): duty cycle, frekuensi.
  - API PWM ESP32: `ledcSetup()`, `ledcAttachPin()`, `ledcWrite()`.
  - LED RGB (Common Cathode): mengontrol warna dengan kombinasi PWM.
- **Praktik:**
  1. Kontrol LED eksternal (nyala/mati + blink pattern).
  2. *LED Dimming*: kecerahan LED naik perlahan kemudian turun (efek *fade*).
  3. **Bonus:** ESP32 menampilkan warna pelangi pada LED RGB dengan PWM.
- **Komponen:** LED merah, hijau, biru; LED RGB; Resistor 220Ω; Breadboard.
- **Capaian Sesi:** Mahasiswa mampu mengontrol output digital dan mengatur intensitas LED via PWM.

---

#### **Pertemuan 4: GPIO Input — Digital & Analog (ADC)**

- **Topik:** Membaca Input dari Perangkat Fisik.
- **Sub-Topik:**
  - Membaca input digital dari *push button* dengan `digitalRead()`.
  - Konsep *debouncing* (software debounce menggunakan `millis()`).
  - Perbandingan Arduino Uno vs ESP32: prosesor 8-bit vs 32-bit dual-core, clock, memori, tegangan 5V vs 3.3V.
  - Pengenalan ADC (*Analog to Digital Converter*): resolusi 12-bit ESP32 (0–4095).
  - Membaca nilai analog: `analogRead()`. Catatan limitasi ADC ESP32 (nonlinear di pin tertentu).
- **Praktik:**
  1. Push button mengontrol LED: satu tombol toggle nyala/mati.
  2. Potensiometer: nilai ADC ditampilkan di Serial Monitor + LED menyala jika melebihi threshold.
  3. *LED Dimming* otomatis berdasarkan putaran potensiometer.
- **Komponen:** Push button, Potensiometer 10kΩ, LED, Resistor.
- **Capaian Sesi:** Mahasiswa memahami input digital/analog dan konsep ADC ESP32.

---

#### **Pertemuan 5: Protokol Komunikasi — UART (ESP32 ↔ Sensor/Modul)**

> **Catatan:** Pada Pertemuan 5–7, fokus protokol komunikasi adalah bagaimana **ESP32 berkomunikasi dengan sensor dan aktuator/modul** (contoh: sensor UART, layar OLED I2C, SD Card SPI). Komunikasi **antar-ESP32** menggunakan protokol yang sama akan dibahas di mata kuliah **IoT Lanjutan**.

- **Topik:** Komunikasi Serial Asinkron (*Universal Asynchronous Receiver-Transmitter*) antara ESP32 dan perangkat/modul.
- **Sub-Topik:**
  - Konsep UART: pin TX, RX, *baud rate*, *start/stop bit*, paritas.
  - ESP32 memiliki 3 port UART (UART0, UART1, UART2).
  - Fungsi: `Serial.begin(baudRate)`, `Serial.print()`, `Serial.println()`, `Serial.available()`, `Serial.read()`, `Serial.readString()`.
  - Komunikasi dua arah: ESP32 ↔ Komputer melalui Serial Monitor.
  - Multi-UART: komunikasi ESP32 ↔ modul GPS / GSM menggunakan `Serial2`.
- **Praktik:**
  1. ESP32 mengirim data sensor (nilai ADC potensiometer) ke Serial Monitor setiap 1 detik.
  2. Komputer mengirim perintah karakter (`'1'` untuk nyalakan LED, `'0'` untuk matikan) ke ESP32.
  3. **Bonus:** Parser perintah string: ESP32 membaca `"LED:ON"` atau `"LED:OFF"` dari Serial Monitor.
- **Komponen:** ESP32 DevKit, LED, Resistor.
- **Capaian Sesi:** Mahasiswa menguasai komunikasi dua arah via UART/Serial antara ESP32 dan modul/komputer.

---

#### **Pertemuan 6: Protokol Komunikasi — I2C (ESP32 ↔ Sensor/Display)**

- **Topik:** Komunikasi ESP32 dengan sensor dan display menggunakan I2C (*Inter-Integrated Circuit*).
- **Sub-Topik:**
  - Arsitektur I2C: Master (ESP32) – Slave (sensor/display), pin SDA (data) dan SCL (clock), *pull-up resistor*.
  - Pengalamatan perangkat I2C (7-bit address, hex). Cara scan alamat dengan `I2C Scanner`.
  - Keunggulan: hemat pin, banyak perangkat pada satu bus (hingga 127 device).
  - Library `Wire.h`: `Wire.begin()`, `Wire.beginTransmission()`, `Wire.write()`, `Wire.read()`.
  - Sensor DHT11/DHT22: koneksi, membaca suhu & kelembapan.
  - Layar OLED 128×64 (SSD1306): library `Adafruit_SSD1306`, menampilkan teks dan grafik.
- **Praktik:**
  1. Scan alamat I2C perangkat (sensor/display) yang terhubung ke ESP32.
  2. Membaca suhu & kelembapan dari DHT22 dan tampilkan di Serial Monitor.
  3. Tampilkan nilai suhu & kelembapan secara *real-time* pada layar OLED 128x64.
- **Komponen:** DHT22, OLED 128×64 (I2C), Resistor pull-up 4.7kΩ.
- **Capaian Sesi:** Mahasiswa mampu menghubungkan dan membaca multiple perangkat I2C dari ESP32.

---

#### **Pertemuan 7: Protokol Komunikasi — SPI (ESP32 ↔ Modul Penyimpanan)**

- **Topik:** Komunikasi ESP32 dengan modul penyimpanan/periferal menggunakan SPI (*Serial Peripheral Interface*).
- **Sub-Topik:**
  - Arsitektur SPI: pin MISO, MOSI, SCLK, CS/SS (*Chip Select*). Komunikasi *full-duplex*.
  - Perbandingan I2C vs SPI: kecepatan, jumlah pin, jarak, penggunaan umum.
  - Library `SPI.h`: `SPI.begin()`, `SPI.transfer()`.
  - Modul SD Card dengan SPI: library `SD.h`. Membuat, membaca, menghapus file.
  - Aplikasi nyata: data logger sensor ke SD Card.
- **Praktik:**
  1. Inisialisasi modul SD Card via SPI, cek format FAT32.
  2. Menulis log data (nilai waktu + suhu DHT22) ke file `log.txt` di SD Card setiap 5 detik.
  3. Membaca isi file `log.txt` dari SD Card dan tampilkan di Serial Monitor.
- **Komponen:** Modul SD Card SPI, MicroSD card, DHT22.
- **Capaian Sesi:** Mahasiswa mampu menggunakan protokol SPI untuk komunikasi ESP32 dengan modul penyimpanan.

---

#### **Pertemuan 8: Sensor — Mengumpulkan Data Lingkungan**

- **Topik:** Membaca berbagai jenis sensor untuk monitoring lingkungan.
- **Sub-Topik:**
  - **Sensor Digital:** Sensor gerak PIR (HC-SR501): logika HIGH/LOW, delay sensitisasi.
  - **Sensor Analog:** LDR (*Light Dependent Resistor*): pembagi tegangan, kalibrasi threshold.
  - **Sensor Jarak:** HC-SR04 (*Ultrasonic*): prinsip kerja *echo timing*, rumus jarak.
  - **Sensor Suhu Presisi:** BMP280 / DS18B20: pembacaan suhu dan tekanan udara.
  - Konsep *sampling rate* dan efisiensi pembacaan sensor.
- **Praktik:**
  1. ESP32 membaca PIR: jika gerakan terdeteksi → LED menyala 5 detik + pesan Serial Monitor.
  2. Sensor LDR: LED otomatis menyala saat gelap (threshold kalibrasi).
  3. HC-SR04: tampilkan jarak dalam cm di OLED. Buzzer berbunyi jika objek < 20cm.
- **Komponen:** PIR HC-SR501, LDR, HC-SR04, Buzzer, OLED 128×64.
- **Capaian Sesi:** Mahasiswa mampu membaca dan merespons data dari berbagai jenis sensor.

---

#### **Pertemuan 9: Aktuator — Memberikan Aksi ke Dunia Fisik**

- **Topik:** Mengendalikan berbagai aktuator untuk merespons perintah atau data sensor.
- **Sub-Topik:**
  - **Buzzer:** Buzzer pasif vs aktif. Menghasilkan nada (tone) dengan PWM.
  - **Servo Motor:** Library `ESP32Servo`. Kontrol sudut 0°–180°, sweep otomatis.
  - **Relay Module:** Prinsip kerja relay elektromekanik. Mengontrol beban AC 220V secara aman. *Active Low* relay.
  - **Motor DC dengan Driver L298N:** Kontrol arah dan kecepatan motor DC menggunakan PWM.
  - Keamanan: isolasi, fuse, wiring yang benar untuk beban tegangan tinggi.
- **Praktik:**
  1. Servo sweep otomatis 0° → 90° → 180° → 90° → 0°.
  2. **Sistem Palang Parkir Otomatis:** HC-SR04 mengukur jarak → jika < 30cm, servo membuka (0°→90°) → setelah 3 detik, servo menutup kembali.
  3. Relay mengontrol LED 220V (atau lampu AC) via push button.
- **Komponen:** Motor Servo SG90, Relay 5V, HC-SR04, Buzzer Pasif.
- **Capaian Sesi:** Mahasiswa mampu mengintegrasikan sensor + aktuator dalam sistem sederhana.

---

#### **Pertemuan 10: Komunikasi WiFi ESP32**

- **Topik:** Menghubungkan ESP32 ke Internet melalui WiFi.
- **Sub-Topik:**
  - Mode WiFi ESP32: **STA** (*Station* — join ke router), **AP** (*Access Point* — buat hotspot sendiri), **STA+AP** (keduanya).
  - Koneksi ke WiFi: `WiFi.begin(ssid, password)`, status koneksi, IP address.
  - Menangani disconnect dan *reconnect* otomatis.
  - Sinkronisasi waktu via **NTP** (*Network Time Protocol*): library `NTPClient` + `WiFiUdp`.
  - Mode AP: membuat hotspot ESP32 untuk konfigurasi awal.
- **Praktik:**
  1. ESP32 terhubung ke WiFi rumah → tampilkan IP dan MAC address di Serial Monitor.
  2. Mode AP: ESP32 membuat hotspot `ESP32-AP`, smartphone bisa terhubung ke hotspot tersebut.
  3. Sinkronisasi waktu dengan NTP → tampilkan jam real-time di OLED.
- **Komponen:** ESP32 DevKit, OLED 128×64.
- **Capaian Sesi:** Mahasiswa mampu menghubungkan ESP32 ke WiFi dan menggunakan NTP.

---

#### **Pertemuan 11: Komunikasi Bluetooth (BLE)**

- **Topik:** Komunikasi nirkabel jarak dekat menggunakan Bluetooth Low Energy.
- **Sub-Topik:**
  - Perbedaan **Bluetooth Classic** vs **BLE** (*Bluetooth Low Energy*): konsumsi daya, range, use case.
  - Arsitektur BLE: **GATT** (*Generic Attribute Profile*), **Service**, **Characteristic**, **UUID**.
  - ESP32 sebagai **BLE Server**: `BLEDevice::init()`, `BLEServer`, `BLEService`, `BLECharacteristic`.
  - ESP32 sebagai **BLE Client**: scan perangkat, konek, baca/tulis characteristic.
  - Aplikasi: monitoring sensor via aplikasi smartphone (nRF Connect / LightBlue).
- **Praktik:**
  1. ESP32 sebagai **BLE Server**: mengirim nilai suhu DHT22 setiap 2 detik ke characteristic BLE.
  2. Baca data dari smartphone menggunakan aplikasi **nRF Connect**.
  3. **Dua arah:** Smartphone mengirim perintah via BLE Write → ESP32 menyalakan/mematikan LED.
- **Komponen:** ESP32 DevKit, DHT22, LED, Smartphone dengan nRF Connect.
- **Capaian Sesi:** Mahasiswa mampu membangun komunikasi dua arah via BLE.

---

#### **Pertemuan 12: Library Populer ESP32**

- **Topik:** Menggunakan pustaka (library) populer untuk mempercepat pengembangan.
- **Sub-Topik:**
  - **WiFiManager** (by tzapu): Konfigurasi WiFi tanpa *hardcode* SSID/Password. ESP32 masuk mode AP jika tidak ada koneksi tersimpan, user pilih WiFi via browser.
  - **ArduinoJson** (v6/v7 by Benoît Blanchon): Serialize dan deserialize format JSON. Parsing respons API, membuat payload.
  - **Preferences** (built-in ESP32): Menyimpan data ke flash (NVS) secara persisten (nama WiFi, konfigurasi, kalibrasI).
  - **PubSubClient**: Library MQTT client untuk ESP32 (preview — dibahas detail di sesi Lanjutan).
  - Cara install library: Library Manager Arduino IDE, `platformio.ini`.
- **Praktik:**
  1. **WiFiManager**: Upload firmware, reset ESP32 → masuk mode AP → hubungkan ke hotspot ESP32 → pilih WiFi rumah via browser → ESP32 menyambung otomatis.
  2. **ArduinoJson**: Parse string JSON `{"suhu":28.5,"lembap":65}` dan cetak nilai masing-masing ke Serial Monitor.
  3. **Preferences**: Simpan counter berapa kali ESP32 di-restart ke flash. Tampilkan saat boot.
- **Komponen:** ESP32 DevKit, Smartphone/PC untuk konfigurasi WiFiManager.
- **Capaian Sesi:** Mahasiswa mampu menggunakan library populer untuk proyek yang lebih profesional.

---

#### **Pertemuan 13: Web Server ESP32**

- **Topik:** ESP32 sebagai peladen web (*Web Server*) lokal.
- **Sub-Topik:**
  - Konsep HTTP: *Request* (GET/POST), *Response*, status code (200, 404).
  - Library `WebServer` (Arduino) atau `ESPAsyncWebServer` (Async, lebih handal).
  - Melayani halaman HTML statis dari kode program (`server.send()`).
  - Menggunakan SPIFFS/LittleFS untuk menyimpan file HTML, CSS, JS di flash ESP32.
  - *REST API* sederhana dari ESP32: endpoint `/sensor` mengembalikan data JSON.
  - *Auto-refresh* halaman web dengan JavaScript (`setInterval + fetch`).
- **Praktik:**
  1. ESP32 melayani halaman web sederhana (HTML + CSS) yang menampilkan nilai suhu & kelembapan dari DHT22.
  2. Tambahkan tombol ON/OFF di halaman web → mengontrol relay/LED ESP32 via HTTP GET.
  3. Tambahkan endpoint `/api/sensor` yang mengembalikan `{"suhu":28.5,"lembap":65}` dalam format JSON.
- **Komponen:** ESP32 DevKit, DHT22, LED/Relay, PC/Smartphone (browser).
- **Capaian Sesi:** Mahasiswa mampu membangun dashboard monitoring berbasis web lokal pada ESP32.

---

#### **Pertemuan 14: Platform IoT — Blynk & MQTT Dasar**

- **Topik:** Menghubungkan ESP32 ke platform IoT cloud untuk monitoring remote.
- **Sub-Topik:**
  - **Blynk IoT (New Blynk 2.0):** Konsep Datastream, Virtual Pin (V0–V255), Widget (Gauge, Button, Chart, Notification).
  - Setup: Buat akun Blynk → Template → Device → Auth Token.
  - Library Blynk: `BLYNK_WRITE(Vx)`, `Blynk.virtualWrite(Vx, value)`.
  - **Pengantar MQTT:** Publish/Subscribe, Topic, Broker (HiveMQ Cloud / broker.hivemq.com public).
  - Library `PubSubClient`: connect ke broker, publish, subscribe, callback.
- **Praktik:**
  1. **Blynk Monitoring:** Kirim data suhu DHT22 ke Gauge widget Blynk → pantau dari smartphone.
  2. **Blynk Kontrol:** Tombol di app Blynk → nyalakan/matikan LED ESP32 dari mana saja.
  3. **MQTT Preview:** ESP32 publish `"Hello from ESP32"` ke topic `iot/dasar/hello` di broker publik, cek di MQTT Explorer.
- **Komponen:** ESP32 DevKit, DHT22, LED, Smartphone.
- **Capaian Sesi:** Mahasiswa mampu menghubungkan ESP32 ke platform IoT cloud.

---

#### **Pertemuan 15: Integrasi Sistem & Persiapan Proyek**

- **Topik:** Menggabungkan semua konsep dan mempersiapkan proyek akhir.
- **Sub-Topik:**
  - Arsitektur sistem IoT lengkap: Sensor → ESP32 → WiFi → Cloud → Dashboard.
  - Praktik baik dalam pengembangan firmware: modularisasi kode, penggunaan `millis()` vs `delay()`, manajemen memori.
  - *Troubleshooting* umum: ESP32 crash (watchdog, stack overflow), WiFi disconnect, sensor error.
  - Briefing proyek akhir: spesifikasi minimal, format presentasi.
  - Diskusi dan konsultasi desain proyek masing-masing mahasiswa/kelompok.
- **Praktik (Workshop):**
  1. Refaktor kode dari sesi sebelumnya menjadi lebih modular (pisahkan fungsi WiFi, sensor, display).
  2. Implementasi *non-blocking code* menggunakan `millis()` untuk multi-task sederhana (baca sensor setiap 2s, update OLED setiap 1s, cek tombol setiap 100ms).
  3. Konsultasi dan validasi desain proyek akhir dengan dosen.
- **Komponen:** Sesuai desain proyek masing-masing.
- **Capaian Sesi:** Mahasiswa memiliki desain proyek akhir yang valid dan kode yang terstruktur.

---

#### **Pertemuan 16: Proyek Akhir — Presentasi & Demo**

- **Topik:** Integrasi seluruh konsep dalam proyek IoT yang fungsional.
- **Spesifikasi Proyek (Minimal):**
  - Menggunakan minimal **2 sensor** berbeda.
  - Menggunakan minimal **1 aktuator**.
  - Terhubung ke **WiFi** dan dapat diakses secara remote (Blynk, Web Server, atau MQTT).
  - Memiliki **antarmuka pengguna** (Web Server atau aplikasi Blynk).

- **Contoh Ide Proyek:**

  | Proyek | Sensor | Aktuator | Konektivitas |
  |---|---|---|---|
  | Sistem Penyiram Tanaman Otomatis | Sensor kelembapan tanah, DHT22 | Pompa air (relay), OLED | Blynk (panel kontrol + notifikasi) |
  | Monitoring Kualitas Udara | MQ-135 (gas), DHT22, BMP280 | Buzzer, LED merah | Web Server ESP32 + grafik real-time |
  | Smart Security Door | PIR, HC-SR04 | Servo (kunci pintu), Buzzer | Blynk (notifikasi + kamera snapshot) |
  | Dashboard Cuaca Lokal | DHT22, BMP280, LDR | OLED + LED RGB | Web Server + NTP |
  | Smart Fish Tank | Sensor suhu DS18B20, LDR | Heater relay, lampu relay | Blynk monitoring + alert |

- **Format Presentasi:**
  1. Demo alat berjalan (5 menit): tunjukkan semua fitur bekerja.
  2. Penjelasan arsitektur & skematik rangkaian (3 menit).
  3. Penjelasan kode utama (3 menit).
  4. Tanya jawab (3 menit).

- **Penilaian:**

  | Kriteria | Bobot |
  |---|---|
  | Fungsionalitas (semua fitur berjalan) | 40% |
  | Kualitas kode (modular, bersih, berkomentar) | 20% |
  | Kreativitas & kompleksitas proyek | 20% |
  | Presentasi & kemampuan menjelaskan | 20% |

---

### **Referensi & Sumber Belajar**

- **Dokumentasi Resmi:** [ESP32 Arduino Core Docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- **Library Reference:** [Arduino Library Reference](https://www.arduino.cc/reference/en/)
- **Tutorial:** [Random Nerd Tutorials - ESP32](https://randomnerdtutorials.com/projects-esp32/)
- **Platform IoT:** [Blynk Documentation](https://docs.blynk.io/)
- **Community:** ESP32 Forum (esp32.com), Arduino subreddit