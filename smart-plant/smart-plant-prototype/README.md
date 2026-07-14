# Smart Plant Prototype

> Sistem penyiraman tanaman otomatis berbasis ESP8266 dengan monitoring kelembapan tanah, kontrol pompa via web dashboard real-time, MQTT, dan integrasi Home Assistant.

**Firmware Version:** 1.0.0  
**Platform:** ESP8266 NodeMCU v3  
**Framework:** Arduino (PlatformIO)

---

## Daftar Isi

1. [Gambaran Umum](#1-gambaran-umum)
2. [Bill of Materials (BOM)](#2-bill-of-materials-bom)
3. [Wiring & Pinout](#3-wiring--pinout)
4. [Struktur Proyek](#4-struktur-proyek)
5. [Arsitektur Software](#5-arsitektur-software)
6. [Modul Firmware](#6-modul-firmware)
   - [6.1 Konfigurasi Compile-time (`config.h`)](#61-konfigurasi-compile-time-configh)
   - [6.2 Konfigurasi Runtime (`app_config`)](#62-konfigurasi-runtime-app_config)
   - [6.3 Sensor Kelembapan Tanah (`sensor`)](#63-sensor-kelembapan-tanah-sensor)
   - [6.4 Kontroler Pompa (`pump`)](#64-kontroler-pompa-pump)
   - [6.5 Display OLED (`display`)](#65-display-oled-display)
   - [6.6 Provisioning WiFi (`wifi_setup`)](#66-provisioning-wifi-wifi_setup)
   - [6.7 MQTT Handler (`mqtt_handler`)](#67-mqtt-handler-mqtt_handler)
   - [6.8 Web Handler (`web_handler`)](#68-web-handler-web_handler)
   - [6.9 Referensi Detail Setiap Fungsi](#69-referensi-detail-setiap-fungsi)
7. [State Machine Pompa](#7-state-machine-pompa)
8. [MQTT — Topik & Payload](#8-mqtt--topik--payload)
9. [HTTP API](#9-http-api)
10. [WebSocket Protocol](#10-websocket-protocol)
11. [Sistem Konfigurasi](#11-sistem-konfigurasi)
12. [Build & Flash](#12-build--flash)
13. [First Boot — Provisioning WiFi](#13-first-boot--provisioning-wifi)
14. [Kalibrasi Sensor](#14-kalibrasi-sensor)
15. [Integrasi Home Assistant](#15-integrasi-home-assistant)
16. [Web Dashboard](#16-web-dashboard)
17. [Troubleshooting](#17-troubleshooting)
18. [Pengembangan Lanjutan](#18-pengembangan-lanjutan)

---

## 1. Gambaran Umum

Smart Plant Prototype adalah firmware IoT untuk menyiram tanaman secara otomatis menggunakan sensor kelembapan kapasitif. Sistem bekerja sepenuhnya **non-blocking** (tidak ada `delay()` di `loop()`) dengan arsitektur event-driven.

**Fitur utama:**
- Penyiraman otomatis berbasis hysteresis dual-threshold (kering < 30%, cukup > 55%)
- State machine pompa dengan safety cutoff dan cooldown
- Web dashboard real-time via WebSocket (tanpa polling)
- MQTT publish/subscribe + Last Will Testament + Home Assistant auto-discovery
- Provisioning WiFi tanpa hardcode via captive portal (WiFiManager)
- Konfigurasi runtime tersimpan di flash (LittleFS JSON)
- OLED display 128×64 dengan layout informatif
- Kontrol manual pompa dari web (5/10/15/20 detik) dan dari MQTT

---

## 2. Bill of Materials (BOM)

| No | Komponen | Spesifikasi | Qty |
|----|----------|-------------|-----|
| 1 | MCU | ESP8266 NodeMCU v3 (LoLin) | 1 |
| 2 | Sensor Kelembapan | Capacitive Soil Moisture Sensor v1.2 (3.3V) | 1 |
| 3 | Sensor Suhu/Kelembapan Udara | DHT22 (AM2302) — 3.3V, akurasi ±0.5°C / ±2% RH | 1 |
| 4 | Relay | 1-channel 5V relay module, active LOW | 1 |
| 5 | Pompa Air | Mini submersible pump 3–6V atau solenoid valve 12V | 1 |
| 6 | OLED Display | SSD1306 0.96" 128×64 I2C | 1 |
| 7 | Power Supply | USB 5V 1A (untuk ESP8266 + relay) | 1 |
| 8 | Resistor pull-up 10kΩ | Untuk DHT22 (jika pakai sensor mentah 4-pin, modul 3-pin sudah onboard) | 1 |
| 9 | Kabel jumper | Dupont male-female | secukupnya |
| 10 | Breadboard / PCB | — | 1 |

> **Catatan:** Relay harus **active LOW** (coil energize saat pin LOW). Module relay yang umum di pasaran (HW-series, SRD-05VDC) sudah active LOW.

---

## 3. Wiring & Pinout

### NodeMCU v3 → Komponen

| Pin NodeMCU | GPIO | Fungsi | Komponen |
|-------------|------|--------|----------|
| D5 | GPIO14 | Relay control (active LOW) | Relay IN |
| A0 | ADC0 | ADC soil sensor | Sensor AOUT |
| **D6** | **GPIO12** | **DHT22 DATA (1-wire, pull-up 10kΩ ke 3V3)** | **DHT22 DATA** |
| D1 | GPIO5 | I2C SCL | OLED SCL |
| D2 | GPIO4 | I2C SDA | OLED SDA |
| 3V3 | — | Power 3.3V | Sensor VCC, **DHT22 VCC** |
| GND | — | Ground | Semua GND |
| VIN / 5V | — | Power 5V | Relay VCC, Pompa (lewat relay) |

### Relay Wiring

```
ESP8266 D5 ──────── Relay IN
GND ─────────────── Relay GND
5V ──────────────── Relay VCC

Relay COM ──── (+) Power pompa
Relay NO  ──── (+) Pompa
(-) Pompa ──── GND power pompa
```

> **Active LOW:** `digitalWrite(D5, LOW)` → relay ON → pompa menyala  
> **Active LOW:** `digitalWrite(D5, HIGH)` → relay OFF → pompa mati

### Rangkaian I2C OLED

```
NodeMCU 3V3 ──── OLED VCC
NodeMCU GND ──── OLED GND
NodeMCU D1  ──── OLED SCL
NodeMCU D2  ──── OLED SDA
```

I2C address default: `0x3C`

### Rangkaian DHT22

DHT22 modul 3-pin (board) — sudah ada pull-up onboard:

```
NodeMCU 3V3 ──── DHT22 VCC (+)
NodeMCU GND ──── DHT22 GND (-)
NodeMCU D6  ──── DHT22 DATA (S)
```

DHT22 sensor mentah 4-pin — pasang resistor pull-up 10kΩ:

```
NodeMCU 3V3 ──┬── DHT22 pin 1 (VCC)
              │
              └─[10kΩ]──┐
                        │
NodeMCU D6  ────────────┴── DHT22 pin 2 (DATA)
                  (pin 3 = NC)
NodeMCU GND ─────────────── DHT22 pin 4 (GND)
```

> **Timing DHT22:** minimal 2 detik antar pembacaan. Firmware memakai interval 2.5 detik (`DHT_READ_INTERVAL`).

---

## 4. Struktur Proyek

```
smart-plant-prototype/
├── platformio.ini              # Konfigurasi build PlatformIO
├── README.md                   # Dokumentasi ini
├── include/
│   ├── config.h                # Konstanta compile-time (pin, timing, topik MQTT)
│   ├── app_config.h            # Struct AppConfig + deklarasi load/save
│   ├── sensor.h                # Class SoilSensor
│   ├── dht_sensor.h            # Class DhtSensor (DHT22)
│   ├── pump.h                  # Class PumpController + enum PumpState
│   ├── display.h               # Class OledDisplay
│   ├── wifi_setup.h            # Fungsi initWifi()
│   ├── mqtt_handler.h          # Class MqttHandler
│   └── web_handler.h           # Class WebHandler
├── src/
│   ├── main.cpp                # Entry point: setup() + loop()
│   ├── app_config.cpp          # Implementasi loadConfig() / saveConfig()
│   ├── sensor.cpp              # Implementasi SoilSensor
│   ├── dht_sensor.cpp          # Implementasi DhtSensor (DHT22)
│   ├── pump.cpp                # Implementasi PumpController state machine
│   ├── display.cpp             # Implementasi OledDisplay
│   ├── wifi_setup.cpp          # Implementasi WiFiManager provisioning
│   ├── mqtt_handler.cpp        # Implementasi MqttHandler
│   └── web_handler.cpp         # Implementasi HTTP server + WebSocket
└── data/
    └── index.html              # Web dashboard (diupload ke LittleFS)
```

---

## 5. Arsitektur Software

### Stack Library

| Library | Versi | Fungsi |
|---------|-------|--------|
| `espressif8266` platform | latest | ESP8266 Arduino framework |
| `Adafruit SSD1306` | ^2.5.7 | Driver OLED display |
| `Adafruit GFX Library` | ^1.11.9 | Primitif grafis (dependen SSD1306) |
| `ArduinoJson` | ^7.0.0 | Serialisasi/deserialisasi JSON |
| `tzapu/WiFiManager` | ^2.0.17 | Captive portal WiFi provisioning |
| `knolleary/PubSubClient` | ^2.8.0 | MQTT client |
| `links2004/WebSockets` | ^2.4.0 | WebSocket server (port 81) |
| `adafruit/DHT sensor library` | ^1.4.6 | Driver DHT22 (suhu + kelembapan udara) |
| `adafruit/Adafruit Unified Sensor` | ^1.1.14 | Dependensi DHT sensor library |
| `ESP8266WebServer` | (bundled) | HTTP server (port 80) — built-in framework |

> **Penting:** `ESPAsyncWebServer` **tidak digunakan** karena menyebabkan konflik enum `HTTP_GET` dengan WiFiManager. `ESP8266WebServer` (synchronous) tidak memiliki konflik ini karena sudah menjadi dependensi WiFiManager itu sendiri.

### Pola Desain

- **Non-blocking loop:** Semua modul menggunakan pattern `millis()` timer, tidak ada `delay()` di `loop()`
- **Event-driven web:** WebSocket server push hanya saat state berubah, bukan polling berkala
- **Callback injection:** `main.cpp` menyuntikkan lambda sebagai callback ke semua modul, mempertahankan separation of concerns
- **Singleton per modul:** Setiap modul adalah satu instance global di `main.cpp`

### Alur Eksekusi `loop()`

```
soilSensor.update()       ← baca ADC setiap 3s (moving average 8 sample)
       ↓
dhtSensor.update()        ← baca DHT22 setiap 2.5s (suhu + kelembapan udara)
       ↓
pump.update(soilPct)      ← jalankan state machine pompa
       ↓
mqtt.update()             ← reconnect jika perlu, panggil client.loop()
       ↓
mqtt.publish(...)         ← publish soil + temp + hum + pump ke broker setiap 10s
       ↓
web.update()              ← handle HTTP request + push WS jika state berubah
       ↓
oled.update(...)          ← refresh OLED setiap 2s
```

---

## 6. Modul Firmware

### 6.1 Konfigurasi Compile-time (`config.h`)

File `include/config.h` mendefinisikan semua konstanta yang **tidak berubah saat runtime**:

| Konstanta | Nilai | Keterangan |
|-----------|-------|------------|
| `PIN_RELAY` | 14 (D5) | GPIO pin relay |
| `PIN_SOIL_ADC` | A0 | Pin ADC sensor |
| `OLED_ADDR` | 0x3C | I2C address OLED |
| `SOIL_RAW_DRY` | 850 | ADC nilai saat sensor di udara kering |
| `SOIL_RAW_WET` | 380 | ADC nilai saat sensor di air penuh |
| `SAMPLE_COUNT` | 8 | Jumlah sampel moving average |
| `SOIL_DRY_THRESHOLD` | 30% | Batas bawah → nyalakan pompa |
| `SOIL_WET_THRESHOLD` | 55% | Batas atas → matikan pompa |
| `PUMP_MIN_ON_MS` | 8000 ms | Pompa minimal menyala 8 detik |
| `PUMP_MAX_ON_MS` | 30000 ms | Safety cutoff 30 detik |
| `COOLDOWN_MS` | 90000 ms | Jeda 90 detik setelah penyiraman |
| `SENSOR_INTERVAL` | 3000 ms | Frekuensi baca sensor |
| `OLED_INTERVAL` | 2000 ms | Frekuensi refresh OLED |
| `MQTT_PUB_INTERVAL` | 10000 ms | Frekuensi publish MQTT |
| `FIRMWARE_VER` | "1.0.0" | Versi firmware |
| `AP_SSID` | "SmartPlant-Proto" | SSID hotspot provisioning |
| `AP_PASSWORD` | "smartplant" | Password hotspot provisioning |

---

### 6.2 Konfigurasi Runtime (`app_config`)

**File:** `include/app_config.h`, `src/app_config.cpp`

`AppConfig` adalah struct yang menyimpan semua parameter yang bisa diubah oleh pengguna tanpa perlu recompile. Disimpan di LittleFS sebagai `/config.json`.

```cpp
struct AppConfig {
  char mqttBroker[64]  = "192.168.1.100";
  int  mqttPort        = 1883;
  char mqttUser[32]    = "";
  char mqttPass[32]    = "";
  int  soilDryRaw      = 850;   // kalibrasi sensor kering
  int  soilWetRaw      = 380;   // kalibrasi sensor basah
  int  dryThreshold    = 30;    // % → mulai pompa
  int  wetThreshold    = 55;    // % → stop pompa
  int  pumpMaxSec      = 30;    // safety cutoff (detik)
  int  cooldownSec     = 90;    // cooldown (detik)
};
```

**Fungsi:**
- `loadConfig(cfg)` → baca `/config.json` dari LittleFS, return `true` jika berhasil
- `saveConfig(cfg)` → tulis semua field ke `/config.json` (format JSON)

**Perilaku saat file tidak ada:** `loadConfig()` return `false`, firmware menggunakan nilai default struct, lalu `saveConfig()` dipanggil untuk membuat file baru.

---

### 6.3 Sensor Kelembapan Tanah (`sensor`)

**File:** `include/sensor.h`, `src/sensor.cpp`

**Class:** `SoilSensor`

**Algoritma Moving Average:**
- Buffer circular 8 sampel (`SAMPLE_COUNT = 8`)
- Saat `begin()`: buffer diisi dengan satu pembacaan ADC awal (hindari nilai aneh di detik pertama)
- Setiap `SENSOR_INTERVAL` (3 detik): baca ADC → masukkan ke buffer → hitung rata-rata

**Pemetaan nilai:**
```
ADC raw → Persentase kelembapan
map(raw, dryRaw, wetRaw, 0, 100)

Contoh (default kalibasi):
  ADC 850 → 0%   (kering)
  ADC 380 → 100% (basah)
  ADC 615 → ~50% (sedang)
```

Hasil di-`constrain(value, 0, 100)` untuk menghindari nilai di luar rentang.

**Deteksi sensor terputus (`isValid()`):**

Pin ADC `A0` yang **floating** (sensor dicabut, kabel putus, short) akan menghasilkan nilai `analogRead()` mendekati 0 atau acak rendah. Karena sensor kapasitif punya logika terbalik (raw rendah = basah), tanpa deteksi nilai 0 akan dimapping menjadi >180% lalu di-clamp ke **100% — ilusi tanah sangat lembap padahal sensor tidak ada**.

Solusi: cek `_raw` apakah dalam rentang kalibrasi ± `SOIL_VALID_MARGIN`:

```cpp
int lo = min(_wetRaw, _dryRaw) - SOIL_VALID_MARGIN;   // default: 380 - 150 = 230
int hi = max(_wetRaw, _dryRaw) + SOIL_VALID_MARGIN;   // default: 850 + 150 = 1000
_valid = (_raw >= lo) && (_raw <= hi);
```

Sensor sehat → raw di [230, 1000] → valid. Sensor lepas → raw ~0 → invalid.

`_recomputeValid()` dipanggil di akhir `begin()`, `update()`, dan `setCalibration()` — flag selalu konsisten dengan raw + kalibrasi terkini.

**Konsekuensi cascade saat `isValid() == false`:**

| Modul | Perilaku |
|-------|----------|
| Pump | Auto-watering DIBLOK; bila sedang `WATERING` → langsung stop ke `COOLDOWN`. `MANUAL_ON` tetap jalan (user explicit). |
| OLED | `Soil: --` (size 2), bar kosong, indikator kanan = `N/C` (No Connection) |
| MQTT | Skip publish topic `soil`. JSON status field `soil: null`. |
| Web/WS | Push `soil: null`. Dashboard tidak append ke `hist[]` — grafik bebas data palsu. |
| Dashboard | Gauge `--`, badge disembunyikan, banner "⚠ Sensor tanah terputus — cek wiring A0". |

**Method:**
| Method | Return | Keterangan |
|--------|--------|------------|
| `begin()` | void | Init pin, pre-fill buffer, recompute valid |
| `update()` | void | Non-blocking, cek timer, recompute valid |
| `getPercent()` | int | Nilai kelembapan 0–100% (cache terakhir, bisa stale jika invalid) |
| `getRaw()` | int | Raw ADC (0–1023) — berguna untuk kalibrasi manual |
| `isValid()` | bool | `true` jika raw dalam rentang [wetRaw, dryRaw] ± `SOIL_VALID_MARGIN` |
| `setCalibration(dryRaw, wetRaw)` | void | Update kalibrasi runtime + recompute valid |

---

### 6.3b Sensor Suhu & Kelembapan Udara — DHT22 (`dht_sensor`)

**File:** `include/dht_sensor.h`, `src/dht_sensor.cpp`

**Class:** `DhtSensor` — wrapper non-blocking sekitar library `adafruit/DHT sensor library` untuk DHT22 di pin **D6 (GPIO12)**.

**Kenapa wrapper:**
- DHT22 punya batasan minimal 2 detik antar pembacaan; bila dipanggil terlalu cepat sensor return NaN.
- Library Adafruit melakukan I/O bit-banging blocking ~25ms per pembacaan — itu OK selama tidak setiap iterasi.
- Wrapper menyimpan state terakhir + flag `valid`, sehingga konsumen (display, MQTT, web) bisa baca nilai kapan saja tanpa memicu I/O ekstra.

**Algoritma:**
- `update()` dipanggil setiap iterasi `loop()`. Dia mengecek `millis() - _lastMs >= DHT_READ_INTERVAL (2500ms)`; bila belum lewat → return.
- Bila lewat → panggil `_dht.readTemperature()` + `_dht.readHumidity()`. Jika salah satu NaN → set `_valid = false`, nilai cache lama tidak diubah (tapi pembaca harus cek `isValid()`).
- Jika valid → simpan ke `_tempC` + `_humPct`, set `_valid = true`.

**Method:**
| Method | Return | Keterangan |
|--------|--------|------------|
| `begin()` | void | Panggil `DHT::begin()`, tunda pembacaan pertama (DHT22 butuh ~1 detik stabil setelah power-on) |
| `update()` | void | Non-blocking, rate-limited oleh `DHT_READ_INTERVAL` |
| `getTemperatureC()` | float | Suhu °C — NAN sebelum pembacaan valid pertama |
| `getHumidity()` | float | Kelembapan udara % — NAN sebelum pembacaan valid pertama |
| `isValid()` | bool | `true` jika pembacaan terakhir sukses |

**Failure mode:** wiring lepas atau pull-up tidak ada → semua pembacaan NaN → `isValid() == false`. Display menunjukkan `--C --%`, MQTT skip publish topik suhu/humidity, dashboard menampilkan banner peringatan.

---

### 6.4 Kontroler Pompa (`pump`)

**File:** `include/pump.h`, `src/pump.cpp`

**Class:** `PumpController`

**Enum State:**
```cpp
enum class PumpState : uint8_t {
  IDLE      = 0,
  WATERING  = 1,
  COOLDOWN  = 2,
  MANUAL_ON = 3
};
```

**Method:**
| Method | Keterangan |
|--------|------------|
| `begin()` | Init pin relay, pastikan pompa mati saat boot |
| `update(soilPct, soilValid)` | Jalankan state machine, panggil setiap loop. `soilValid=false` → stop WATERING auto + blok auto-start (MANUAL_ON tetap jalan) |
| `forceOn(durationMs)` | Paksa pompa ON selama N ms (mode manual) |
| `forceOff()` | Paksa pompa OFF, masuk COOLDOWN |
| `isOn()` | Return `true` jika relay aktif |
| `getState()` | Return `PumpState` saat ini |
| `getStateStr()` | Return string state: "IDLE"/"WATERING"/"COOLDOWN"/"MANUAL" |
| `setThresholds(dry, wet)` | Update threshold runtime |
| `setTimings(minMs, maxMs, cooldownMs)` | Update timing runtime |

**Safety — sensor terputus:**

Jika `soilValid == false`, `update()` melakukan dua hal:
1. Bila state saat ini `WATERING`: paksa stop relay → masuk `COOLDOWN`. Tidak ada feedback untuk tahu kapan harus berhenti, lebih aman cut early.
2. Bila state `IDLE`: auto-start (`soilPct < dryThreshold`) dilewati. Pompa hanya bisa nyala via `forceOn()` (manual).

`MANUAL_ON` sengaja tidak terpengaruh — user mengaktifkan pompa secara explicit dan punya tombol Matikan di UI.

**Implementasi relay:**
```cpp
// Active LOW: LOW = coil energized = pompa ON
digitalWrite(PIN_RELAY, on ? LOW : HIGH);
```

---

### 6.5 Display OLED (`display`)

**File:** `include/display.h`, `src/display.cpp`

**Class:** `OledDisplay` — driver SSD1306 128×64 px via I2C

**Layout Layar Utama:**
```
┌──────────────────────────────┐
│ SmartPlant Proto    (●)(●)   │  y=0  — judul + dot WiFi + dot MQTT
├──────────────────────────────┤  y=9  — garis separator
│ Soil:  65%                   │  y=12 — label (size1) + nilai (size2)
│ [████████████░░░░░░]  P:ON   │  y=29 — progress bar 82×7 px
│ WATER...            27C 60%  │  y=39 — pump state (kiri) + DHT22 (kanan)
├──────────────────────────────┤  y=47 — garis separator
│ IP: 192.168.1.55             │  y=50 — alamat IP
│ MQTT: OK                     │  y=58 — status MQTT
└──────────────────────────────┘
```

**Format DHT22 di OLED (kanan baris pump state):**
- Valid: `<temp>C <hum>%` (integer, mis. `27C 60%`)
- Invalid: `--C --%`

**Format Soil di OLED saat sensor terputus (`soilValid == false`):**
- Nilai: `Soil: --` (placeholder, tanpa `%`)
- Bar: kosong (0% fill)
- Indikator kanan: `N/C` (No Connection) menggantikan `P:ON`/`P:OFF`

**Keterangan dot status (pojok kanan atas):**
- Dot kiri (x=113): WiFi — solid = connected, outline = disconnected
- Dot kanan (x=124): MQTT — solid = connected, outline = disconnected

**Mode layar khusus:**
- `showBoot(msg)` — ditampilkan selama proses inisialisasi
- `showAPMode(ssid)` — ditampilkan saat masuk captive portal WiFiManager

**Method:**
| Method | Keterangan |
|--------|------------|
| `begin()` | Init SSD1306, return false jika gagal |
| `update(soilPct, soilValid, tempC, humPct, dhtValid, pumpOn, state, ip, wifiOk, mqttOk)` | Refresh non-blocking (2s); render DHT22 di baris pump state; saat `soilValid=false` render `Soil: --` + bar kosong + `N/C` |
| `showBoot(msg)` | Tampilkan pesan boot |
| `showAPMode(ssid)` | Tampilkan info AP mode |

---

### 6.6 Provisioning WiFi (`wifi_setup`)

**File:** `include/wifi_setup.h`, `src/wifi_setup.cpp`

**Fungsi:** `initWifi(cfg, apCallback)`

Menggunakan library **WiFiManager (tzapu)** untuk provisioning WiFi tanpa hardcode credentials.

**Alur:**
1. WiFiManager mencoba connect ke WiFi tersimpan
2. Jika gagal/belum ada → buka hotspot `SmartPlant-Proto` (password: `smartplant`)
3. Pengguna connect ke hotspot, buka browser → `192.168.4.1`
4. Isi form: SSID WiFi + password + parameter MQTT
5. Klik Save → WiFiManager connect ke WiFi, firmware lanjut

**Parameter MQTT di captive portal:**

| Label | Field `AppConfig` | Default |
|-------|-------------------|---------|
| MQTT Broker IP | `mqttBroker` | 192.168.1.100 |
| MQTT Port | `mqttPort` | 1883 |
| MQTT User | `mqttUser` | (kosong) |
| MQTT Password | `mqttPass` | (kosong) |

**Timeout AP mode:** 180 detik (3 menit) → jika tidak ada yang connect, ESP8266 restart otomatis.

**Setelah connect:** Nilai yang diisi di captive portal disalin ke `AppConfig` dan disimpan ke flash.

---

### 6.7 MQTT Handler (`mqtt_handler`)

**File:** `include/mqtt_handler.h`, `src/mqtt_handler.cpp`

**Class:** `MqttHandler`

**Arsitektur:**
- `WiFiClient _wifi` + `PubSubClient _client{_wifi}` — dua objek terpisah
- Static `std::function<void(bool)> _pumpCb` + plain C callback `mqttMessageCb()` — workaround keterbatasan PubSubClient yang tidak menerima lambda dengan capture

**Inisialisasi:**
```cpp
mqtt.begin(appCfg, [](bool on) {
  if (on) pump.forceOn(5000);  // MQTT default = 5 detik
  else    pump.forceOff();
});
```

**Reconnect logic (`_reconnect()`):**
```
1. Cek cooldown 5 detik antar percobaan
2. _wifi.stop() + yield()  ← PENTING: tutup TCP socket lama
3. _client.connect() dengan LWT ke topic availability
4. Jika OK: publish "online", subscribe pump/set, kirim HA discovery (sekali saja)
5. Jika gagal: log rc=N ke Serial
```

> **Kenapa `_wifi.stop()`?** — Setelah disconnect, `WiFiClient` di ESP8266 bisa tersangkut di state `CLOSE_WAIT`/`FIN_WAIT`. Tanpa menutup socket secara eksplisit, `PubSubClient::connect()` gagal dengan `rc=-2` (MQTT_CONNECT_FAILED). Solusi: `_wifi.stop()` + `yield()` sebelum mencoba connect ulang.

**Publish (`publish()`):**
- Throttled ke `MQTT_PUB_INTERVAL` (10 detik)
- Semua pesan bersifat **retained** sehingga broker menyimpan nilai terakhir

**Method:**
| Method | Keterangan |
|--------|------------|
| `begin(cfg, pumpCb)` | Setup server, buffer, callback |
| `update()` | Reconnect + `client.loop()` |
| `publish(soilPct, soilValid, tempC, humPct, dhtValid, pumpOn, state)` | Publish soil (skip topic bila `soilValid=false`) + DHT22 (skip bila `dhtValid=false`) + pump ke broker (throttled) |
| `isConnected()` | Return `true` jika connected (non-const karena PubSubClient) |

---

### 6.8 Web Handler (`web_handler`)

**File:** `include/web_handler.h`, `src/web_handler.cpp`

**Class:** `WebHandler`

**Dua server dalam satu class:**
- `ESP8266WebServer _server{80}` — HTTP port 80
- `WebSocketsServer _ws{81}` — WebSocket port 81

**Change Detection (`update()`):**
Server melacak nilai sebelumnya dan hanya push WebSocket jika ada perubahan:
```cpp
bool changed = (soil      != _prevSoil)
            || (soilValid != _prevSoilValid) // toggle valid/invalid memicu push
            || (tInt      != _prevTemp)      // temp & hum dibandingkan dalam int*10
            || (hInt      != _prevHum)       // (1 desimal) untuk hindari float drift
            || (pump      != _prevPump)
            || (mqtt      != _prevMqtt)
            || (strcmp(state, _prevState) != 0)
            || (millis() - _lastPushMs >= 30000UL);  // heartbeat 30s
```

**Payload status JSON (push WS, type `"s"`):**
```json
{
  "t": "s",
  "soil": 65,
  "temp": 27.3,
  "humidity": 60.1,
  "pump": false,
  "state": "IDLE",
  "uptime": 5025,
  "mqtt": true,
  "ip": "192.168.1.55",
  "ver": "1.0.0"
}
```
> `soil` bernilai `null` bila `SoilSensor::isValid() == false` (sensor terputus). `temp`/`humidity` bernilai `null` bila pembacaan DHT22 invalid.

**Pump command dari WS:**
```json
{"cmd": "pump", "state": "on", "dur": 5000}
```
- `dur` di-clamp ke rentang 1000–120000 ms
- Setelah command dieksekusi, server langsung broadcast status terbaru

---

## 6.9 Referensi Detail Setiap Fungsi

Bagian ini mendokumentasikan setiap fungsi dalam firmware secara lengkap: signature, parameter, nilai kembalian, dan penjelasan cara kerja internal.

---

### `app_config.cpp`

---

#### `loadConfig(AppConfig& cfg)` → `bool`

```cpp
bool loadConfig(AppConfig& cfg);
```

**Tujuan:** Membaca file `/config.json` dari LittleFS dan mengisi struct `AppConfig` dengan nilai yang tersimpan.

**Parameter:**
- `cfg` *(AppConfig&)* — struct konfigurasi yang akan diisi. Nilai yang sudah ada di struct digunakan sebagai **fallback** jika key tidak ditemukan di JSON (operator `|`).

**Return value:** `true` jika file ditemukan dan JSON berhasil di-parse. `false` jika file tidak ada, tidak bisa dibuka, atau JSON rusak. Saat return `false`, struct `cfg` tidak dimodifikasi.

**Alur eksekusi:**
1. Cek `LittleFS.exists(CONFIG_FILE)` — jika tidak ada, langsung return `false`.
2. Buka file dengan `LittleFS.open(CONFIG_FILE, "r")`.
3. Parse JSON dengan `deserializeJson(doc, f)`.
4. Tutup file segera setelah parsing (sebelum assign nilai).
5. Assign setiap field menggunakan operator `|` (contoh: `doc["mqttPort"] | cfg.mqttPort`) — jika key ada di JSON, gunakan nilainya; jika tidak, pertahankan nilai existing.
6. Return `true`.

**Dipanggil dari:** `setup()` di `main.cpp`, langkah ke-3 urutan inisialisasi.

---

#### `saveConfig(const AppConfig& cfg)` → `bool`

```cpp
bool saveConfig(const AppConfig& cfg);
```

**Tujuan:** Menyimpan seluruh isi `AppConfig` ke file `/config.json` di LittleFS (menimpa file lama).

**Parameter:**
- `cfg` *(const AppConfig&)* — struct konfigurasi sumber data yang akan disimpan.

**Return value:** `true` jika file berhasil dibuat dan ditulis. `false` jika LittleFS gagal membuka file untuk write.

**Alur eksekusi:**
1. Buka file dengan mode `"w"` (write, buat jika belum ada, timpa jika sudah ada).
2. Buat `JsonDocument` dan isi semua 10 field dari struct `cfg`.
3. `serializeJson(doc, f)` — tulis JSON ke file secara langsung (tidak di-buffer ke RAM).
4. Tutup file.
5. Return `true`.

**Dipanggil dari:**
- `setup()` setelah WiFiManager selesai (simpan MQTT params yang diisi via captive portal).
- `WebHandler` route `POST /api/config` setelah menerima konfigurasi baru.

---

### `sensor.cpp`

---

#### `SoilSensor::begin()` → `void`

```cpp
void SoilSensor::begin();
```

**Tujuan:** Inisialisasi sensor dengan membaca ADC sekali dan mengisi seluruh circular buffer, sehingga `getPercent()` langsung valid tanpa menunggu 8 siklus (SAMPLE_COUNT).

**Parameter:** Tidak ada.

**Alur eksekusi:**
1. `analogRead(PIN_SOIL_ADC)` — baca ADC pertama kali.
2. Isi semua 8 slot `_buf[0..7]` dengan nilai yang sama.
3. Set `_raw` ke nilai pembacaan tersebut.
4. Hitung `_pct` awal: `constrain(map(first, _dryRaw, _wetRaw, 0, 100), 0, 100)`.

**Alasan pre-fill buffer:** Jika buffer diinisialisasi dengan 0, rata-rata akan sangat rendah (misal 106 dari 8 × 850 ÷ 8 saat kering) sampai 8 pembacaan nyata terkumpul. Pre-fill memastikan nilai langsung representatif.

**Dipanggil dari:** `setup()`.

---

#### `SoilSensor::update()` → `void`

```cpp
void update();
```

**Tujuan:** Fungsi non-blocking yang melakukan satu pembacaan ADC baru setiap `SENSOR_INTERVAL` (3000 ms), lalu memperbarui moving average dan persentase kelembaban.

**Parameter:** Tidak ada. Menggunakan internal state `_lastMs`, `_idx`, `_buf`, `_raw`, `_pct`.

**Alur eksekusi:**
1. **Guard timer:** `if (millis() - _lastMs < SENSOR_INTERVAL) return;` — keluar segera jika belum waktunya.
2. `_lastMs = millis()` — catat waktu pembacaan.
3. `_buf[_idx] = analogRead(PIN_SOIL_ADC)` — simpan ke posisi saat ini di circular buffer.
4. `_idx = (_idx + 1) % SAMPLE_COUNT` — majukan indeks (wrap-around ke 0 setelah 7).
5. Hitung rata-rata semua elemen buffer dengan loop dan pembagian integer.
6. Konversi `_raw` ke persentase dengan `map(_raw, _dryRaw, _wetRaw, 0, 100)`.
7. Clamp hasil ke [0, 100] dengan `constrain()`.
8. Panggil `_recomputeValid()` — set flag `_valid` berdasarkan apakah `_raw` ada dalam rentang [wetRaw, dryRaw] ± `SOIL_VALID_MARGIN`.

**Mengapa moving average?** ADC ESP8266 memiliki noise yang signifikan (±5–10 ADC counts). Moving average 8 sampel meratakan noise ini sehingga pompa tidak menyala/mati karena spike sesaat.

**Dipanggil dari:** `loop()`, setiap iterasi.

---

#### `SoilSensor::isValid()` → `bool`

```cpp
bool isValid() const;
```

**Tujuan:** Apakah pembacaan sensor terakhir berasal dari sensor yang benar-benar terpasang dan sehat (bukan ADC floating / kabel putus / short).

**Return value:** `_valid` — flag yang di-recompute setiap kali `_raw` berubah (di `begin()`, `update()`, `setCalibration()`).

**Logika deteksi:**
- Sensor capacitive sehat menghasilkan ADC raw di rentang ~[380, 850] (default kalibrasi `wetRaw`–`dryRaw`).
- Pin `A0` floating saat sensor dicabut → `analogRead()` ~0 atau acak rendah → di luar rentang.
- Tanpa deteksi: raw 0 di-mapping ke `map(0, 850, 380, 0, 100) ≈ 180%` lalu di-clamp ke **100%**, menampilkan tanah "lembap penuh" yang palsu.

```cpp
int lo = min(_wetRaw, _dryRaw) - SOIL_VALID_MARGIN;
int hi = max(_wetRaw, _dryRaw) + SOIL_VALID_MARGIN;
_valid = (_raw >= lo) && (_raw <= hi);
```

Penggunaan `min/max` agar logika tetap benar jika user mengganti orientasi kalibrasi (mis. pakai sensor resistif di masa depan dengan `wetRaw > dryRaw`).

**Margin (`SOIL_VALID_MARGIN = 150`):** dipilih supaya:
- Tanah ekstrem kering (raw mendekati 850–900) tetap dianggap valid.
- Air keruh / sensor terendam lebih dalam (raw mendekati 350–400) tetap valid.
- Floating ADC (0–100) atau short ke VCC (1000+) tetap di luar rentang → invalid.

**Konsumen:**
- `PumpController::update()` — kalau invalid: stop watering, blok auto-start.
- `OledDisplay::update()` — kalau invalid: render `Soil: --` + `N/C`.
- `MqttHandler::publish()` — kalau invalid: skip topic `soil` + JSON `soil: null`.
- `WebHandler::_pushStatus()` — kalau invalid: JSON `soil: null` (dashboard tampilkan banner).

---

#### `SoilSensor::_recomputeValid()` → `void` *(private)*

```cpp
void _recomputeValid();
```

**Tujuan:** Update flag `_valid` berdasarkan `_raw` saat ini dan kalibrasi `_wetRaw`/`_dryRaw`.

**Dipanggil dari:** `begin()` (setelah pre-fill buffer), `update()` (setelah moving average), `setCalibration()` (karena rentang valid bergantung pada kalibrasi).

---

#### `SoilSensor::getPercent()` → `int`

```cpp
int getPercent() const;
```

**Tujuan:** Getter untuk nilai kelembaban terakhir yang sudah dikonversi ke persentase.

**Return value:** Integer 0–100. Nilai sudah di-clamp, tidak bisa di luar rentang ini.

> **Catatan:** Nilai cache terakhir tetap dikembalikan meskipun `isValid()` saat ini `false`. Konsumen wajib cek `isValid()` sebelum mempercayai nilai — kalau tidak, akan terlihat angka stale (terakhir saat sensor masih terpasang).

**Dipanggil dari:** `loop()` → `pump.update()`, `mqtt.publish()`, `web.update()`, `oled.update()`.

---

#### `SoilSensor::getRaw()` → `int`

```cpp
int getRaw() const;
```

**Tujuan:** Getter untuk nilai ADC rata-rata mentah (sebelum konversi ke %).

**Return value:** Integer 0–1023, merupakan rata-rata dari 8 pembacaan ADC terakhir.

**Kegunaan:** Digunakan untuk keperluan kalibrasi — catat nilai saat sensor kering dan saat sensor basah untuk mengisi `soilDryRaw`/`soilWetRaw`.

---

#### `SoilSensor::setCalibration(int dryRaw, int wetRaw)` → `void`

```cpp
void setCalibration(int dryRaw, int wetRaw);
```

**Tujuan:** Mengatur nilai kalibrasi ADC yang digunakan oleh fungsi `update()` untuk konversi raw → persen.

**Parameter:**
- `dryRaw` *(int)* — nilai ADC saat sensor benar-benar kering (di udara). Biasanya 800–900.
- `wetRaw` *(int)* — nilai ADC saat sensor terendam air. Biasanya 350–420.

**Catatan inversi:** Sensor kapasitif menghasilkan nilai ADC **lebih tinggi saat kering** dan **lebih rendah saat basah** (kebalikan dari sensor resistif). Fungsi `map()` menangani inversi ini secara natural karena `dryRaw > wetRaw`, sehingga output 0% saat `dryRaw` dan 100% saat `wetRaw`.

**Side effect:** Memanggil `_recomputeValid()` setelah update kalibrasi — rentang valid bergantung pada `_wetRaw`/`_dryRaw`.

**Dipanggil dari:** `setup()` setelah `loadConfig()` dengan nilai dari `appCfg.soilDryRaw` dan `appCfg.soilWetRaw`.

---

### `dht_sensor.cpp`

---

#### `DhtSensor::begin()` → `void`

```cpp
void DhtSensor::begin();
```

**Tujuan:** Inisialisasi library DHT (`_dht.begin()`) dan menunda pembacaan pertama sampai `DHT_READ_INTERVAL` terlewati di iterasi `loop()` berikutnya.

**Parameter:** Tidak ada. Internal state: `_dht{PIN_DHT22, DHT22}` (di-init di anggota class, pin = D6 / GPIO12).

**Alur eksekusi:**
1. `_dht.begin()` — set pin sebagai input, init internal timing library Adafruit.
2. `_lastMs = millis()` — sengaja set ke waktu sekarang agar `update()` pertama menunggu `DHT_READ_INTERVAL` (~2.5 detik) sebelum melakukan I/O pertama. Tujuannya: DHT22 butuh ~1 detik setelah power-on untuk stabil; menunda pembacaan menghindari pembacaan NaN di siklus pertama.

**Alasan tidak pre-fill seperti SoilSensor:** DHT22 bukan ADC instan — bit-banging library Adafruit blocking ~25 ms per call. Pre-fill berarti memblokir setup() sebanyak 8× 25 ms = 200 ms. Lebih baik konsumen menerima NaN selama ~2.5 detik pertama dan cek `isValid()`.

**Dipanggil dari:** `setup()` di `main.cpp`, setelah `soilSensor.begin()` dan `soilSensor.setCalibration()`.

---

#### `DhtSensor::update()` → `void`

```cpp
void DhtSensor::update();
```

**Tujuan:** Non-blocking — melakukan satu pembacaan DHT22 (suhu + kelembapan) setiap `DHT_READ_INTERVAL` (2500 ms), lalu memperbarui cache nilai dan flag `_valid`.

**Parameter:** Tidak ada. Internal state: `_lastMs`, `_tempC`, `_humPct`, `_valid`.

**Alur eksekusi:**
1. **Guard timer:** `if (millis() - _lastMs < DHT_READ_INTERVAL) return;` — keluar segera jika belum waktunya. Penting karena DHT22 spec **minimum 2 detik antar pembacaan**; baca lebih cepat → return NaN.
2. `_lastMs = millis()` — catat waktu.
3. `t = _dht.readTemperature()` — blocking ~25 ms, bit-bang 1-wire protocol DHT22, return °C atau NaN.
4. `h = _dht.readHumidity()` — blocking ~25 ms, return % RH atau NaN.
5. Jika **salah satu** `isnan(t)` atau `isnan(h)` → set `_valid = false` dan return tanpa update `_tempC`/`_humPct`. Konsumen wajib cek `isValid()`.
6. Jika kedua valid → simpan ke `_tempC` + `_humPct`, set `_valid = true`.

**Performance note:** total ~50 ms blocking per pembacaan setiap 2.5 detik = ~2% utilisasi CPU. Acceptable untuk firmware non-RTOS.

**Dipanggil dari:** `loop()` di `main.cpp`, segera setelah `soilSensor.update()`.

---

#### `DhtSensor::getTemperatureC()` → `float`

```cpp
float DhtSensor::getTemperatureC() const;
```

**Tujuan:** Return suhu terakhir dalam derajat Celsius.

**Return value:** `_tempC` — nilai cache pembacaan valid terakhir, atau `NAN` jika belum pernah berhasil baca.

**Penting:** Konsumen harus cek `isValid()` atau `isnan(returnValue)` sebelum format/transmit. Display menampilkan `--C` jika invalid; MQTT skip publish.

---

#### `DhtSensor::getHumidity()` → `float`

```cpp
float DhtSensor::getHumidity() const;
```

**Tujuan:** Return kelembapan udara terakhir dalam persen (0–100% RH).

**Return value:** `_humPct` — sama semantik dengan `getTemperatureC()`. `NAN` sebelum pembacaan valid pertama.

---

#### `DhtSensor::isValid()` → `bool`

```cpp
bool DhtSensor::isValid() const;
```

**Tujuan:** Apakah pembacaan DHT22 terakhir berhasil (kedua suhu + kelembapan return non-NaN).

**Return value:** `_valid`.

**Use case:**
- Display: pilih `printf("%2dC %2d%%", t, h)` vs `print("--C --%")`.
- MQTT: kalau false, skip publish ke topik `temp`/`humidity` agar Home Assistant tidak melihat nilai NaN.
- Web JSON: serialize `null` ke field `temp`/`humidity` agar dashboard menampilkan placeholder + banner peringatan.

---

### `pump.cpp`

---

#### `PumpController::begin()` → `void`

```cpp
void begin();
```

**Tujuan:** Inisialisasi pin relay dan pastikan pompa dalam kondisi OFF saat booting.

**Alur eksekusi:**
1. `pinMode(PIN_RELAY, OUTPUT)` — set pin D5 sebagai output digital.
2. `_setRelay(false)` — kirim sinyal HIGH ke pin (relay active LOW → coil tidak aktif → pompa OFF).

**Penting:** Selalu matikan relay eksplisit di `begin()`. Jika tidak, pin GPIO bisa dalam kondisi floating saat boot (terutama saat upload firmware), berpotensi menyalakan pompa secara tidak sengaja.

**Dipanggil dari:** `setup()`.

---

#### `PumpController::_setRelay(bool on)` → `void` *(private)*

```cpp
void _setRelay(bool on);
```

**Tujuan:** Fungsi wrapper internal yang mengatur pin relay dengan logika inversi (active LOW).

**Parameter:**
- `on` *(bool)* — `true` = nyalakan pompa, `false` = matikan pompa.

**Alur eksekusi:**
1. `_relayOn = on` — simpan status internal.
2. `digitalWrite(PIN_RELAY, on ? LOW : HIGH)` — inversi: `true` → LOW (relay energize), `false` → HIGH (relay de-energize).

**Mengapa wrapper?** Memusatkan logika inversi di satu tempat. Semua kode yang ingin mengubah relay memanggil `_setRelay(true/false)` dengan semantik yang intuitif, tanpa perlu ingat bahwa hardware-nya active LOW.

---

#### `PumpController::update(int soilPct, bool soilValid)` → `void`

```cpp
void update(int soilPct, bool soilValid);
```

**Tujuan:** Inti logika state machine pompa. Menentukan kapan pompa harus menyala/mati berdasarkan kelembaban tanah dan timer. Juga melakukan safety check sensor terputus.

**Parameter:**
- `soilPct` *(int)* — nilai kelembaban tanah saat ini (0–100), didapat dari `SoilSensor::getPercent()`.
- `soilValid` *(bool)* — `SoilSensor::isValid()` — `false` berarti sensor lepas / ADC floating / kabel putus. Pembacaan `soilPct` tidak dapat dipercaya.

**Safety pre-check (sebelum switch state):**

Jika `!soilValid` DAN state saat ini `WATERING` (auto): paksa stop relay → masuk `COOLDOWN`, return. Alasan: tanpa sensor tidak ada feedback "kapan harus berhenti", bisa overwatering. `MANUAL_ON` sengaja tidak ikut dihentikan karena user explicit menekan tombol "Nyalakan".

**Alur eksekusi per state:**

**`IDLE`:**
- Jika **`soilValid`** DAN `soilPct < _dryThreshold`: nyalakan relay (`_setRelay(true)`), catat `_stateStart`, transisi ke `WATERING`.
- Selain itu (termasuk `soilValid == false`): tidak ada aksi, pompa tetap OFF.

**`WATERING`:**
- Hitung `elapsed = millis() - _stateStart`.
- Jika `elapsed >= _maxOnMs`: safety cutoff — matikan relay, masuk `COOLDOWN`. Log ke Serial.
- Jika `soilPct > _wetThreshold` DAN `elapsed >= _minOnMs`: tanah cukup basah dan sudah menyala minimal `minOnMs` — matikan relay, masuk `COOLDOWN`. Log ke Serial.
- Jika tidak memenuhi kondisi apapun: pompa tetap menyala, tunggu.

**`COOLDOWN`:**
- Jika `elapsed >= _cooldownMs`: transisi ke `IDLE`. Log ke Serial.
- Selain itu: diam, pompa tetap OFF.

**`MANUAL_ON`:**
- Jika `millis() >= _manualEndMs`: durasi manual selesai — matikan relay, masuk `COOLDOWN`.
- Selain itu: pompa tetap menyala sampai timer habis. **Tidak terpengaruh `soilValid`** — user explicit minta nyala.

**Mengapa `PUMP_MIN_ON_MS`?** Sensor kelembaban membutuhkan waktu untuk bereaksi setelah pompa menyiram. Tanpa minimum ON time, pompa bisa mati dalam hitungan detik karena sensor belum mendeteksi perubahan (propagasi air ke sensor membutuhkan waktu).

**Dipanggil dari:** `loop()`.

---

#### `PumpController::forceOn(unsigned long durationMs)` → `void`

```cpp
void forceOn(unsigned long durationMs = 10000UL);
```

**Tujuan:** Override state machine — paksa pompa menyala selama durasi tertentu, terlepas dari kondisi sensor.

**Parameter:**
- `durationMs` *(unsigned long)* — durasi menyala dalam milidetik. Default 10000 (10 detik). Dari web UI diklem 1000–120000. Dari MQTT default 5000.

**Alur eksekusi:**
1. `_setRelay(true)` — nyalakan pompa segera.
2. `_manualEndMs = millis() + durationMs` — hitung kapan harus berhenti.
3. `_stateStart = millis()` — catat waktu mulai untuk tracking.
4. `_state = PumpState::MANUAL_ON`.

**Catatan:** Dapat dipanggil dari state apapun — termasuk saat `WATERING` atau `COOLDOWN`. Ini adalah hard override.

**Dipanggil dari:**
- Callback MQTT di `main.cpp`: `if (on) pump.forceOn(5000)`.
- Callback WebSocket di `WebHandler::begin()` saat menerima `{cmd:"pump", state:"on", dur:N}`.

---

#### `PumpController::forceOff()` → `void`

```cpp
void forceOff();
```

**Tujuan:** Hentikan pompa segera dan masuk ke `COOLDOWN` (bukan `IDLE`).

**Alur eksekusi:**
1. `_setRelay(false)` — matikan relay segera.
2. `_stateStart = millis()`.
3. `_state = PumpState::COOLDOWN`.

**Mengapa masuk `COOLDOWN` bukan `IDLE`?** Agar pompa tidak langsung menyala lagi jika kondisi tanah masih kering. Jika langsung ke `IDLE`, pompa bisa menyala ulang dalam hitungan milidetik.

**Dipanggil dari:**
- Callback MQTT: `else pump.forceOff()`.
- Callback WebSocket saat `state == "off"`.

---

#### `PumpController::isOn()` → `bool`

```cpp
bool isOn() const;
```

**Return value:** Status relay saat ini. `true` = relay aktif (pompa menyala), `false` = relay tidak aktif.

---

#### `PumpController::getState()` → `PumpState`

```cpp
PumpState getState() const;
```

**Return value:** Enum `PumpState` saat ini (`IDLE`, `WATERING`, `COOLDOWN`, atau `MANUAL_ON`).

---

#### `PumpController::getStateStr()` → `const char*`

```cpp
const char* getStateStr() const;
```

**Return value:** String literal statis sesuai state: `"IDLE"`, `"WATERING"`, `"COOLDOWN"`, atau `"MANUAL"`. `"UNKNOWN"` untuk state yang tidak dikenal (defensive default).

**Dipanggil dari:** `mqtt.publish()`, `web.update()`, `oled.update()`.

---

#### `PumpController::msSinceChange()` → `unsigned long`

```cpp
unsigned long msSinceChange() const;
```

**Return value:** Selisih waktu antara `millis()` sekarang dengan `_stateStart` (timestamp transisi state terakhir) dalam milidetik.

---

#### `PumpController::setThresholds(int dryPct, int wetPct)` → `void`

```cpp
void setThresholds(int dryPct, int wetPct);
```

**Tujuan:** Update threshold kelembaban dari konfigurasi runtime (menggantikan default dari `config.h`).

**Parameter:**
- `dryPct` — kelembaban di bawah ini akan menyalakan pompa. Default: `SOIL_DRY_THRESHOLD` (30).
- `wetPct` — kelembaban di atas ini akan menghentikan pompa. Default: `SOIL_WET_THRESHOLD` (55).

**Dipanggil dari:** `setup()` dengan nilai `appCfg.dryThreshold` dan `appCfg.wetThreshold`.

---

#### `PumpController::setTimings(unsigned long, unsigned long, unsigned long)` → `void`

```cpp
void setTimings(unsigned long minOnMs, unsigned long maxOnMs, unsigned long cooldownMs);
```

**Tujuan:** Update parameter timing state machine dari konfigurasi runtime.

**Parameter:**
- `minOnMs` — durasi minimum pompa menyala (default `PUMP_MIN_ON_MS` = 8000 ms).
- `maxOnMs` — durasi maximum safety cutoff (default `PUMP_MAX_ON_MS` = 30000 ms).
- `cooldownMs` — durasi cooldown setelah penyiraman (default `COOLDOWN_MS` = 90000 ms).

**Dipanggil dari:** `setup()` dengan `PUMP_MIN_ON_MS`, `appCfg.pumpMaxSec*1000`, `appCfg.cooldownSec*1000`.

---

### `display.cpp`

---

#### `OledDisplay::begin()` → `bool`

```cpp
bool begin();
```

**Tujuan:** Inisialisasi driver SSD1306 via I2C dan persiapkan display untuk digunakan.

**Return value:** `true` jika OLED ditemukan dan berhasil diinisialisasi. `false` jika tidak ada respons di alamat I2C (periksa wiring atau alamat I2C).

**Alur eksekusi:**
1. `_disp.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)` — inisialisasi dengan internal charge pump 3.3V (tidak butuh supply VCC eksternal untuk LCD voltage).
2. Jika gagal, return `false` segera.
3. `_disp.clearDisplay()` — kosongkan frame buffer di RAM.
4. `_disp.setTextColor(SSD1306_WHITE)` — set warna default.
5. `_disp.display()` — kirim frame buffer kosong ke layar (bersihkan artefak sisa).
6. Return `true`.

**Catatan:** Gagalnya `begin()` bersifat **non-fatal** bagi firmware. Semua fungsi display lain tetap akan dipanggil tapi tidak akan menampilkan apapun.

**Dipanggil dari:** `setup()`, langkah pertama.

---

#### `OledDisplay::showBoot(const char* msg)` → `void`

```cpp
void showBoot(const char* msg);
```

**Tujuan:** Menampilkan layar status boot dengan pesan yang dapat diubah. Digunakan untuk memberikan feedback visual selama setiap tahap inisialisasi.

**Parameter:**
- `msg` *(const char*)* — string status yang ditampilkan (contoh: `"Init..."`, `"WiFi..."`, `"Ready!"`).

**Layout yang dihasilkan:**
```
Smart Plant
Prototype v1.0.0
────────────────
[msg]
```

**Alur eksekusi:**
1. Bersihkan display.
2. Set text size 1, cursor (0,0).
3. Tulis `"Smart Plant"` dan `"Prototype v1.0.0"` (menggunakan F() macro untuk string di flash).
4. Gambar garis horizontal di y=17.
5. Set cursor (0,22) dan tulis `msg`.
6. `_disp.display()` — tampilkan.

**Dipanggil dari:** `setup()` sebelum setiap tahap inisialisasi utama.

---

#### `OledDisplay::showAPMode(const char* ssid)` → `void`

```cpp
void showAPMode(const char* ssid);
```

**Tujuan:** Menampilkan instruksi WiFi setup mode untuk memandu pengguna mengonfigurasi WiFi via captive portal.

**Parameter:**
- `ssid` *(const char*)* — nama SSID Access Point yang harus dihubungi pengguna.

**Layout yang dihasilkan:**
```
WiFi Setup Mode
───────────────
Hubungkan ke:
SmartPlant-Proto
192.168.4.1
Pass: smartplant
```

**Dipanggil dari:** Callback `apCallback` yang diteruskan ke `initWifi()` — dipanggil saat WiFiManager membuka AP mode.

---

#### `OledDisplay::_drawBar(int x, int y, int w, int h, int pct)` → `void` *(private)*

```cpp
void _drawBar(int x, int y, int w, int h, int pct);
```

**Tujuan:** Menggambar progress bar horizontal dengan outline dan fill sesuai persentase.

**Parameter:**
- `x`, `y` — koordinat piksel pojok kiri atas.
- `w`, `h` — lebar dan tinggi total inklusif border (piksel).
- `pct` — nilai pengisian 0–100%.

**Alur eksekusi:**
1. `drawRect(x, y, w, h, WHITE)` — gambar outline/border.
2. Hitung lebar fill: `constrain(map(pct, 0, 100, 0, w-2), 0, w-2)`. Dikurangi 2 untuk menyisakan 1 piksel border di kanan dan kiri.
3. Jika `fill > 0`: `fillRect(x+1, y+1, fill, h-2, WHITE)` — isi area dalam border.

**Dipanggil dari:** `OledDisplay::update()` untuk menggambar bar kelembaban.

---

#### `OledDisplay::update(int soilPct, bool soilValid, float tempC, float humPct, bool dhtValid, bool pumpOn, PumpState pumpState, const char* ip, bool wifiOk, bool mqttOk)` → `void`

```cpp
void update(int soilPct, bool soilValid,
            float tempC, float humPct, bool dhtValid,
            bool pumpOn, PumpState pumpState,
            const char* ip, bool wifiOk, bool mqttOk);
```

**Tujuan:** Refresh tampilan OLED dengan status terkini. Non-blocking — hanya melakukan rendering aktual setiap `OLED_INTERVAL` (2000 ms).

**Parameter:**
- `soilPct` — kelembaban tanah (0–100%).
- `soilValid` — `SoilSensor::isValid()`. Bila `false`, render `Soil: --` + bar kosong + `N/C`.
- `tempC` — suhu dari DHT22 (°C, bisa NaN).
- `humPct` — kelembapan udara dari DHT22 (%, bisa NaN).
- `dhtValid` — `true` jika pembacaan DHT22 terakhir berhasil. Bila `false`, tampilkan `--C --%`.
- `pumpOn` — status relay.
- `pumpState` — state pompa (untuk tampilkan string state yang tepat).
- `ip` — IP address sebagai C-string.
- `wifiOk` — `true` jika WiFi terhubung (untuk dot indikator).
- `mqttOk` — `true` jika MQTT terhubung (untuk dot indikator).

**Alur eksekusi:**
1. Guard timer: jika `millis() - _lastMs < OLED_INTERVAL`, return.
2. Update `_lastMs`.
3. `clearDisplay()`.
4. **Title bar (y=0):** Tulis `"SmartPlant Proto"`. Gambar lingkaran solid jika WiFi OK, outline jika tidak (x=113). Sama untuk MQTT (x=124).
5. **Separator:** Garis horizontal di y=9.
6. **Kelembaban (y=12–35):**
   - Label `"Soil:"` size 1.
   - Bila `soilValid` → nilai `soilPct%` size 2. Bila tidak → `--` size 2 (tanpa `%`).
   - `_drawBar(0, 29, 82, 7, soilValid ? soilPct : 0)` — bar kosong saat invalid (hindari ilusi 100% lembap).
   - Indikator kanan (x=88, y=30):
     - `N/C` (No Connection) bila `!soilValid` — paling diprioritaskan.
     - `P:ON` / `P:OFF` bila valid.
7. **Baris kombinasi (y=39):**
   - Kiri (x=0): switch-case state pompa — string dipendekkan agar muat (`IDLE` / `WATER...` / `COOLDOWN` / `MANUAL`).
   - Kanan (x=86): jika `dhtValid` → `printf("%2dC %2d%%", round(tempC), round(humPct))`. Jika tidak → `"--C --%"`. Suhu/hum di-clamp ke [0,99] untuk jaga lebar tetap 7 karakter.
8. **Separator:** Garis horizontal di y=47.
9. **Network info:** IP di y=50, status MQTT di y=58.
10. `display()` — kirim buffer ke layar.

**Dipanggil dari:** `loop()`.

---

### `wifi_setup.cpp`

---

#### `onAPStarted(WiFiManager*)` → `void` *(static internal)*

```cpp
static void onAPStarted(WiFiManager*);
```

**Tujuan:** Plain C callback yang didaftarkan ke `wm.setAPCallback()`. Dipanggil oleh WiFiManager saat AP mode dimulai.

**Alur:** Panggil `_apCb(AP_SSID)` jika `_apCb` tidak null. `_apCb` adalah lambda yang disimpan dari `initWifi()` yang pada akhirnya memanggil `oled.showAPMode()`.

**Mengapa perlu fungsi terpisah?** `WiFiManager::setAPCallback()` menerima function pointer biasa (`void(*)(WiFiManager*)`), bukan `std::function`. Lambda dengan capture tidak bisa dikonversi ke function pointer, sehingga perlu wrapper static dengan variabel static `_apCb`.

---

#### `initWifi(AppConfig& cfg, std::function<void(const char*)> apCallback)` → `bool`

```cpp
bool initWifi(AppConfig& cfg, std::function<void(const char*)> apCallback = nullptr);
```

**Tujuan:** Menghubungkan ESP8266 ke WiFi menggunakan WiFiManager. Jika belum ada kredensial tersimpan, buka captive portal.

**Parameter:**
- `cfg` *(AppConfig&)* — konfigurasi yang berisi MQTT params yang sudah ada (sebagai pre-fill) dan akan diupdate dengan nilai yang diisi user.
- `apCallback` *(std::function)* — lambda dipanggil saat AP mode dimulai. Menerima SSID string. Digunakan untuk `oled.showAPMode()`.

**Return value:** `true` jika berhasil terhubung ke WiFi. `false` jika timeout (3 menit tanpa koneksi).

**Alur eksekusi:**
1. Simpan `apCallback` ke `_apCb` static.
2. Buat instance `WiFiManager wm`, set callback AP.
3. Buat 4 `WiFiManagerParameter` untuk MQTT fields — pre-filled dengan nilai dari `cfg`.
4. Tambahkan semua parameter ke `wm`.
5. `wm.setConfigPortalTimeout(180)` — timeout 3 menit.
6. `wm.autoConnect(AP_SSID, AP_PASSWORD)`:
   - Jika ada kredensial WiFi tersimpan di flash ESP8266: coba connect langsung.
   - Jika tidak ada / gagal: buka AP `"SmartPlant-Proto"` pw `"smartplant"`, tunggu user isi form di browser.
7. Jika `connected == true`: copy nilai 4 parameter MQTT dari WiFiManager ke `cfg`.
8. Return `connected`.

**Dipanggil dari:** `setup()`.

---

#### `getLocalIP()` → `String`

```cpp
String getLocalIP();
```

**Tujuan:** Helper untuk mendapatkan IP address lokal ESP8266.

**Return value:** `String` berisi IP address (contoh: `"192.168.1.55"`), atau `"0.0.0.0"` jika belum terhubung.

---

### `mqtt_handler.cpp`

---

#### `mqttMessageCb(char* topic, byte* payload, unsigned int len)` → `void` *(static internal)*

```cpp
static void mqttMessageCb(char* topic, byte* payload, unsigned int len);
```

**Tujuan:** Callback yang dipanggil PubSubClient setiap kali pesan MQTT masuk ke topic yang di-subscribe.

**Parameter:**
- `topic` — C-string nama topic.
- `payload` — byte array isi pesan (tidak null-terminated).
- `len` — panjang payload dalam byte.

**Alur eksekusi:**
1. Konversi `topic` ke `String t`.
2. Konversi `payload` + `len` ke `String msg`, lalu `.trim()` (hapus whitespace/newline).
3. Log ke Serial.
4. Jika `t == MQTT_TOPIC_PUMP_SET` dan `_pumpCb` tidak null:
   - Lowercase `msg`.
   - Jika `"on"` / `"true"` / `"1"`: `_pumpCb(true)` → pompa menyala.
   - Jika `"off"` / `"false"` / `"0"`: `_pumpCb(false)` → pompa mati.

**Dipanggil dari:** Library PubSubClient secara internal saat ada pesan masuk.

---

#### `MqttHandler::begin(AppConfig& cfg, std::function<void(bool)> pumpCallback)` → `void`

```cpp
void begin(AppConfig& cfg, std::function<void(bool)> pumpCallback);
```

**Tujuan:** Konfigurasi awal MQTT client — set server, buffer size, dan callback.

**Parameter:**
- `cfg` *(AppConfig&)* — digunakan untuk `mqttBroker` dan `mqttPort`.
- `pumpCallback` *(std::function\<void(bool)\>)* — dipanggil dengan `true` untuk nyalakan pompa, `false` untuk matikan.

**Alur eksekusi:**
1. Simpan pointer `_cfg = &cfg`.
2. Simpan callback ke static `_pumpCb`.
3. `_client.setServer(cfg.mqttBroker, cfg.mqttPort)`.
4. `_client.setBufferSize(512)` — default 256 byte tidak cukup untuk payload HA discovery (~400 byte).
5. `_client.setCallback(mqttMessageCb)`.

**Catatan:** `begin()` tidak langsung connect ke broker. Connect terjadi di `update()` → `_reconnect()`.

**Dipanggil dari:** `setup()`.

---

#### `MqttHandler::update()` → `void`

```cpp
void update();
```

**Tujuan:** Loop handler MQTT — maintain koneksi dan proses pesan masuk.

**Alur eksekusi:**
1. Jika `WiFi.status() != WL_CONNECTED`: return (tidak ada gunanya tanpa WiFi).
2. Jika `!_client.connected()`: panggil `_reconnect()`, lalu return (tidak panggil `loop()` di iterasi yang sama dengan reconnect).
3. `_client.loop()` — proses pesan masuk, kirim PINGREQ keepalive, proses PUBLISH/SUBACK yang pending.

**Dipanggil dari:** `loop()`.

---

#### `MqttHandler::publish(int soilPct, bool soilValid, float tempC, float humPct, bool dhtValid, bool pumpOn, PumpState state)` → `void`

```cpp
void publish(int soilPct, bool soilValid,
             float tempC, float humPct, bool dhtValid,
             bool pumpOn, PumpState state);
```

**Tujuan:** Publish data sensor dan status lengkap ke broker MQTT. Dieksekusi maksimal setiap `MQTT_PUB_INTERVAL` (10 detik).

**Parameter:**
- `soilPct` — kelembaban tanah (0–100).
- `soilValid` — bila `false`, topic `soil` di-skip dan field `soil` di JSON status diserialize sebagai `null`.
- `tempC` / `humPct` — suhu (°C) dan kelembapan udara (%) dari DHT22.
- `dhtValid` — bila `false`, topic `temp` & `humidity` di-skip dan field `temp`/`humidity` di JSON status diserialize sebagai `null`.
- `pumpOn` — status relay.
- `state` — state pompa (dikonversi ke integer untuk JSON).

**Alur eksekusi (jika connected dan interval terpenuhi):**
1. **Hanya jika `soilValid`:** Publish `soilPct` sebagai string ke `smartplant/proto/soil` (retained=true). Saat invalid topic dilewati biar HA tidak mencatat 0% atau 100% palsu dari ADC floating.
2. **Hanya jika `dhtValid`:** Publish `tempC` (1 desimal) ke `smartplant/proto/temp` dan `humPct` (1 desimal) ke `smartplant/proto/humidity`. Gunakan `dtostrf()` untuk format float di ESP8266 (Arduino `printf` default tidak include `%f`). Kalau invalid → topic dilewati supaya HA tidak melihat NaN sebagai pembacaan valid.
3. Publish `"ON"` atau `"OFF"` ke `smartplant/proto/pump/state` (retained=true).
4. Buat `JsonDocument` dengan semua field status (termasuk `soil`/`temp`/`humidity` atau `null` sesuai validitas), serialize ke buffer `char buf[320]`.
5. Publish JSON ke `smartplant/proto/status` (retained=true).

**Flag retained:** Semua pesan retained agar broker menyimpan nilai terakhir. Klien baru (misal HA restart) langsung mendapat state terkini tanpa menunggu publish berikutnya.

**Dipanggil dari:** `loop()`.

---

#### `MqttHandler::isConnected()` → `bool`

```cpp
bool isConnected();
```

**Return value:** `_client.connected()` — `true` jika sesi MQTT aktif.

**Catatan:** Tidak bisa `const` karena `PubSubClient::connected()` bukan const method (keterbatasan library).

---

#### `MqttHandler::_reconnect()` → `void` *(private)*

```cpp
void _reconnect();
```

**Tujuan:** Mencoba membangun ulang koneksi MQTT dengan backoff minimal 5 detik antar percobaan.

**Alur eksekusi:**
1. Guard: jika `millis() - _lastAttemptMs < 5000`, return.
2. Update `_lastAttemptMs`.
3. **`_wifi.stop()` + `yield()`** — tutup paksa socket TCP lama. Ini menyelesaikan bug ESP8266 di mana socket bisa stuck di `CLOSE_WAIT`/`FIN_WAIT` setelah disconnect, yang menyebabkan `_client.connect()` return `rc=-2` (network error bukan MQTT error).
4. Tentukan mode connect:
   - Jika `cfg.mqttUser` tidak kosong: `_client.connect(ID, user, pass, LWT_topic, 0, true, "offline")`.
   - Jika kosong: `_client.connect(ID, nullptr, nullptr, LWT_topic, 0, true, "offline")`.
   - LWT: saat device disconnect tidak normal, broker otomatis publish `"offline"` ke availability topic.
5. Jika connect berhasil:
   - Publish `"online"` ke availability topic.
   - `_client.subscribe(MQTT_TOPIC_PUMP_SET)`.
   - Jika `!_discoveryDone`: panggil `_publishHADiscovery()`, set `_discoveryDone = true`.
6. Jika gagal: log `rc=N` (kode error PubSubClient).

---

#### `MqttHandler::_publishHADiscovery()` → `void` *(private)*

```cpp
void _publishHADiscovery();
```

**Tujuan:** Mempublish payload konfigurasi auto-discovery Home Assistant untuk dua entitas, memungkinkan HA mengenali device secara otomatis tanpa konfigurasi YAML.

**Alur eksekusi (untuk setiap entitas):**

**Entitas 1 — Soil Moisture Sensor:**
- Topic publish: `homeassistant/sensor/smartplant_proto_soil/config`
- Payload JSON mencakup: `name`, `unique_id`, `state_topic`, `unit_of_measurement` (`%`), `device_class` (`moisture`), `availability_topic`, `payload_available/not_available`, dan object `device` (identifiers, name, model, manufacturer, sw_version).

**Entitas 2 — Temperature Sensor (DHT22):**
- Topic publish: `homeassistant/sensor/smartplant_proto_temp/config`
- Payload: `device_class: "temperature"`, `unit_of_measurement: "°C"`, `state_class: "measurement"`, state_topic = `smartplant/proto/temp`. Object `device` mengacu ke `identifiers: smartplant_proto` yang sama supaya HA mengelompokkan semua entitas di bawah satu perangkat fisik.

**Entitas 3 — Humidity Sensor (DHT22):**
- Topic publish: `homeassistant/sensor/smartplant_proto_humidity/config`
- Payload: `device_class: "humidity"`, `unit_of_measurement: "%"`, `state_class: "measurement"`, state_topic = `smartplant/proto/humidity`. `device.identifiers` sama dengan di atas.

**Entitas 4 — Pompa Switch:**
- Topic publish: `homeassistant/switch/smartplant_proto_pump/config`
- Payload JSON mencakup: `name`, `unique_id`, `state_topic`, `command_topic`, `payload_on/off`, `state_on/off`, `availability_topic`, dan object `device` yang sama.

Semua payload dipublish dengan flag **retained = true** agar HA membaca konfigurasi ini saat restart.

**Dipanggil dari:** `_reconnect()`, hanya sekali (`_discoveryDone` flag).

---

### `web_handler.cpp`

---

#### `WebHandler::begin(...)` → `void`

```cpp
void begin(AppConfig& cfg,
           std::function<int()>               getSoil,
           std::function<bool()>              getSoilValid,
           std::function<float()>             getTempC,
           std::function<float()>             getHumPct,
           std::function<bool()>              getPumpOn,
           std::function<const char*()>       getPumpState,
           std::function<void(unsigned long)> pumpForceOn,
           std::function<void()>              pumpForceOff,
           std::function<bool()>              getMqttOk);
```

**Tujuan:** Inisialisasi HTTP server dan WebSocket server dengan mendaftarkan semua routes dan event handler.

**Parameter:** Semua callback (lambda) yang disuntikkan dari `main.cpp`:
- `cfg` — referensi konfigurasi untuk dibaca/ditulis route `/api/config`.
- `getSoil` — getter kelembaban tanah.
- `getSoilValid` — getter `SoilSensor::isValid()`. Bila `false`, JSON push field `soil: null`.
- `getTempC` — getter suhu DHT22 (float, NaN jika invalid).
- `getHumPct` — getter kelembapan udara DHT22 (float, NaN jika invalid).
- `getPumpOn` — getter status relay.
- `getPumpState` — getter string state pompa.
- `pumpForceOn(ms)` — perintah nyalakan pompa dengan durasi `ms`.
- `pumpForceOff` — perintah matikan pompa.
- `getMqttOk` — getter status MQTT.

**Routes HTTP yang didaftarkan:**

| Method | Path | Perilaku |
|--------|------|---------|
| GET | `/` | Buka `index.html` dari LittleFS dengan `streamFile()` |
| GET | `/api/config` | Serialize 9 field `AppConfig` ke JSON, kirim sebagai response |
| POST | `/api/config` | Parse JSON body, update field yang ada, `saveConfig()`, return `{"ok":true}` |
| `*` | `*` | 404 Not Found |

**WebSocket events:**
- `WStype_CONNECTED`: log client, panggil `_pushStatus(clientNum)` untuk kirim state awal.
- `WStype_DISCONNECTED`: log.
- `WStype_TEXT`: parse JSON, jika `cmd=="pump"` dan `state=="on"`, clamp `dur` ke 1000–120000 ms, panggil `_pumpForceOn(dur)`. Jika `state=="off"`, panggil `_pumpForceOff()`. Lalu `_pushStatus()`.

**Alur akhir:** `_ws.begin()` → `_server.begin()`.

**Dipanggil dari:** `setup()`.

---

#### `WebHandler::update()` → `void`

```cpp
void update();
```

**Tujuan:** Proses HTTP request dan push update WebSocket jika ada perubahan state.

**Alur eksekusi:**
1. `_server.handleClient()` — proses satu HTTP request jika ada di antrian.
2. `_ws.loop()` — proses WebSocket frames, kirim PING, handle reconnect internal.
3. Jika `_ws.connectedClients() == 0`, return (tidak ada yang perlu dikirim).
4. Ambil nilai terkini semua state via callback.
5. Deteksi perubahan: bandingkan dengan `_prev*` values.
6. Cek heartbeat: `millis() - _lastPushMs >= 30000`.
7. Jika `changed || heartbeat`: update semua `_prev*`, update `_lastPushMs`, panggil `_pushStatus()`.

**Heartbeat 30 detik:** Memastikan koneksi WebSocket tetap aktif dan client mendapat state terkini meski tidak ada perubahan (mencegah timeout idle dan sinkronisasi ulang jika ada desync).

**Dipanggil dari:** `loop()`.

---

#### `WebHandler::_pushStatus(int clientNum)` → `void` *(private)*

```cpp
void _pushStatus(int clientNum = -1);
```

**Tujuan:** Buat JSON status dan kirim via WebSocket.

**Parameter:**
- `clientNum` — jika `-1`: broadcast ke semua client via `_ws.broadcastTXT()`. Jika `>= 0`: kirim hanya ke client tertentu via `_ws.sendTXT()`.

**Payload JSON yang dikirim:**
```json
{
  "t":        "s",
  "soil":     65,
  "temp":     27.3,
  "humidity": 60.1,
  "pump":     false,
  "state":    "IDLE",
  "uptime":   3600,
  "mqtt":     true,
  "ip":       "192.168.1.55",
  "ver":      "1.0.0"
}
```

- Field `t: "s"` adalah type discriminator — client mengabaikan pesan dengan `t` selain `"s"`.
- `uptime` dalam detik (`millis() / 1000`).
- `soil` di-set `null` jika `SoilSensor::isValid() == false`; dashboard menampilkan `--`, badge disembunyikan, banner "⚠ Sensor tanah terputus" muncul, dan data tidak di-append ke `hist[]` (grafik tetap bersih).
- `temp` & `humidity` di-set `null` jika `DhtSensor::isValid() == false` atau hasil getter NaN; dashboard akan menampilkan `--` + banner peringatan.

---

### `main.cpp`

---

#### `setup()` → `void`

```cpp
void setup();
```

**Tujuan:** Entry point inisialisasi firmware Arduino — dipanggil sekali saat booting.

**Urutan inisialisasi:**

| # | Kode | Alasan urutan |
|---|------|---------------|
| 1 | `Serial.begin(115200)` | Debug output tersedia sejak awal |
| 2 | `oled.begin()` | OLED ready untuk menampilkan boot progress |
| 3 | `LittleFS.begin()` | Filesystem harus mount sebelum `loadConfig` |
| 4 | `loadConfig(appCfg)` | Config harus loaded sebelum apply ke sensor/pump |
| 5 | `soilSensor.begin()` + `setCalibration()` | Init sensor dengan kalibrasi dari config |
| 5 | `dhtSensor.begin()` | Init DHT22 — pembacaan pertama ditunda 2.5 detik |
| 5 | `pump.begin()` + `setThresholds()` + `setTimings()` | Init relay + apply threshold/timing |
| 6 | `initWifi(appCfg, cb)` | WiFi harus connected sebelum MQTT/Web bisa jalan |
| 7 | `saveConfig(appCfg)` | Simpan MQTT params yang mungkin baru diisi via portal |
| 8 | `mqtt.begin(appCfg, cb)` | MQTT init setelah WiFi connected |
| 9 | `web.begin(appCfg, ...)` | Web server start terakhir, setelah semua data tersedia |

**Penanganan kegagalan:**
- Jika `LittleFS.begin()` gagal: warning di OLED + Serial, firmware tetap lanjut (web server tidak aktif tapi sensor/pompa/MQTT tetap berjalan).
- Jika `initWifi()` return `false` (timeout 3 menit): `ESP.restart()` — restart dan coba lagi.

---

#### `loop()` → `void`

```cpp
void loop();
```

**Tujuan:** Loop utama Arduino — dipanggil secepat mungkin, berulang-ulang.

**Prinsip desain:** Tidak ada `delay()`. Setiap modul memiliki timer internal sendiri dan langsung return jika belum waktunya bekerja. Ini memungkinkan semua subsistem berjalan kooperatif dalam satu thread tanpa blocking.

**Urutan panggilan dan frekuensi efektif:**

| Panggilan | Frekuensi Efektif | Keterangan |
|-----------|-------------------|------------|
| `soilSensor.update()` | Setiap 3 detik | Baca ADC + update moving average |
| `dhtSensor.update()` | Setiap 2.5 detik | Baca DHT22 (blocking ~50 ms saat tick aktif) |
| `pump.update(soilPct)` | Setiap iterasi | State machine, hampir tidak ada overhead saat tidak ada transisi |
| `mqtt.update()` | Setiap iterasi | `_client.loop()` ~1 ms, reconnect hanya jika perlu |
| `mqtt.publish(...)` | Setiap 10 detik | Publish soil + DHT22 + pump ke broker |
| `web.update()` | Setiap iterasi | Handle HTTP request jika ada, push WS jika berubah |
| `oled.update(...)` | Setiap 2 detik | Render OLED — operasi I2C ~5–10 ms |

---

### `index.html` (JavaScript)

---

#### `wsConnect()` → `void`

**Tujuan:** Membuka koneksi WebSocket ke firmware dan mendaftarkan semua event handler.

**URL:** `ws://<hostname>:81/` — hostname diambil otomatis dari `location.hostname` (IP device yang diakses browser).

**Event handlers:**
- `onopen`: update pill ke "Terhubung" (hijau), hentikan animasi blink, reset `reconnDelay` ke 1000 ms.
- `onmessage`: parse JSON string menjadi object, teruskan ke `onMsg()`. Error parsing diabaikan silently dengan `try/catch`.
- `onclose`: update pill ke "Terputus" (kuning), mulai animasi blink, jadwalkan `wsConnect()` ulang setelah `reconnDelay` ms, lalu `reconnDelay *= 1.6` (max 16000 ms — exponential backoff).
- `onerror`: panggil `ws.close()` untuk trigger `onclose` handler.

---

#### `wsSend(o)` → `void`

**Tujuan:** Mengirim objek JavaScript sebagai pesan JSON ke WebSocket server.

**Parameter:** `o` — object JavaScript yang akan di-`JSON.stringify()`.

**Guard:** Hanya kirim jika `ws && ws.readyState === 1` (WebSocket.OPEN). Mencegah error jika koneksi sedang terputus.

---

#### `onMsg(d)` → `void`

**Tujuan:** Handler utama untuk pesan status dari firmware. Memperbarui seluruh UI.

**Parameter:** `d` — object hasil `JSON.parse()` dari payload WebSocket.

**Filter:** Abaikan jika `d.t !== 's'` (type bukan status).

**Operasi UI yang diperbarui:**
1. Label versi di header (`verLabel`).
2. Gauge SVG kelembaban via `setGauge(d.soil)` — handle `null` (sensor terputus).
3. **Hanya jika `d.soil != null`:** push ke array `hist[]`, trim jika melebihi `MAXH` (60 data), update counter "N data". Bila `null`, data tidak di-append agar grafik tidak terkontaminasi nilai palsu saat sensor lepas.
4. Render ulang chart via `drawChart()`.
5. Update card DHT22 via `setDht(d.temp, d.humidity)`.
6. Update UI pompa via `setPump(d.pump, d.state)`.
7. Update chip MQTT: class `ok`/`err`, teks `"Terhubung"`/`"Terputus"`.
8. Update IP address dan uptime.

---

#### `setDht(t, h)` → `void`

**Tujuan:** Render card "Suhu & Kelembapan Udara" dari payload DHT22.

**Parameter:**
- `t` *(number | null)* — suhu °C, atau `null` bila pembacaan invalid.
- `h` *(number | null)* — kelembapan udara %, atau `null` bila invalid.

**Cara kerja:**
- Validasi tiap nilai: `null`/`undefined`/`isNaN` → tampilkan `--`.
- Bila valid: format `toFixed(1)` (mis. `27.3`) ke elemen `tempVal` / `humVal`.
- **Hint warna kontekstual** (cosmetic):
  - Suhu < 22 → kotak suhu kelas `cool` (biru).
  - Suhu > 30 → kotak suhu kelas `warm` (kuning).
  - Hum ≥ 60 → kotak kelembapan kelas `humid` (hijau).
- Banner `dhtStatus` ditampilkan bila salah satu nilai invalid (peringatan wiring D6).

---

#### `setGauge(v)` → `void`

**Tujuan:** Animasi gauge SVG kelembaban berdasarkan nilai persentase, atau tampilkan placeholder + banner peringatan saat sensor terputus.

**Parameter:** `v` *(number | null)* — kelembaban 0–100, atau `null` bila `SoilSensor::isValid() == false`.

**Handling sensor terputus (`v === null` atau `undefined`):**
- Reset arc gauge ke kosong (`strokeDashoffset = 503`) + warna abu `#cbd5b8`.
- Set teks tengah ke `--` warna abu.
- Set deskripsi ke "Sensor tanah terputus".
- Sembunyikan badge OPTIMAL/KERING/LEMBAB.
- Tampilkan banner `#soilStatus`: "⚠ Sensor tanah terputus — cek wiring A0".
- Return early — tidak menjalankan logika klasifikasi warna.

**Cara kerja SVG gauge:**
- Lingkaran SVG `r=80`, keliling ≈ 503 px.
- `stroke-dasharray: 503` (total keliling).
- `stroke-dashoffset: 503 * (1 - v/100)` — semakin kecil offset, semakin penuh lingkaran.
- Offset 503 = 0% (tidak ada stroke), offset 0 = 100% (penuh).
- Transisi CSS `0.9s cubic-bezier` memberikan animasi halus.

**Threshold warna:**
- `v < 30` → oranye (`#f0a020`), badge `KERING`.
- `30 ≤ v < 55` → hijau (`#22a852`), badge `OPTIMAL`.
- `v ≥ 55` → biru (`#3498db`), badge `LEMBAB`.

---

#### `setPump(on, state)` → `void`

**Tujuan:** Sinkronisasi UI panel pompa dengan state dari firmware.

**Parameter:**
- `on` *(bool)* — apakah pompa sedang aktif.
- `state` *(string)* — string state dari firmware: `"IDLE"`, `"WATERING"`, `"COOLDOWN"`, atau `"MANUAL_ON"`.

**Cara kerja:**
1. Lookup `state` di map `SM` untuk mendapatkan `dot` class, `name`, dan `sub` text.
2. Update class CSS `stateDot` (mengontrol animasi: `idle`=abu, `watering`=pulse hijau, `cooldown`=oranye, `manual`=pulse biru).
3. Update teks `stateName` dan `stateSub`.
4. `btnOn.disabled = on` (tombol ON aktif hanya saat pompa mati).
5. `btnOff.disabled = !on` (tombol OFF aktif hanya saat pompa menyala).

---

#### `setDur(btn, ms)` → `void`

**Tujuan:** Mengubah durasi manual pompa yang dipilih.

**Parameter:**
- `btn` — elemen DOM button yang diklik.
- `ms` — durasi dalam milidetik (5000, 10000, 15000, atau 20000).

**Cara kerja:** Simpan `ms` ke `selDur`, hapus class `sel` dari semua `.dur-btn`, tambahkan `sel` ke `btn` yang diklik (visual highlight).

---

#### `pumpOn()` → `void`

**Tujuan:** Mengirim perintah nyalakan pompa ke firmware via WebSocket dengan durasi yang dipilih.

**Cara kerja:** `wsSend({cmd:'pump', state:'on', dur:selDur})` + `toast('info', '...')`.

---

#### `pumpOff()` → `void`

**Tujuan:** Mengirim perintah matikan pompa ke firmware via WebSocket.

**Cara kerja:** `wsSend({cmd:'pump', state:'off'})` + `toast('warn', '...')`.

---

#### `drawChart()` → `void`

**Tujuan:** Render grafik riwayat kelembaban di elemen `<canvas id="chart">`.

**Cara kerja:**
1. Ambil lebar canvas dari `offsetWidth` (responsif terhadap ukuran window).
2. Bersihkan canvas, isi background.
3. **Grid:** Gambar garis putus-putus horizontal di 25%, 50%, 75% dengan label persen.
4. **Threshold lines:** Gambar garis batas kering (`cfg.dryThreshold`, oranye) dan basah (`cfg.wetThreshold`, biru) sebagai garis putus-putus.
5. Jika data < 2 titik, stop (tidak bisa gambar garis).
6. **Koordinat:** Untuk setiap titik data, hitung `x = P.l + i * (cW/(n-1))`, `y = P.t + cH - (v/100)*cH`.
7. **Area fill:** Gambar path dari semua titik, tutup ke bawah, isi dengan gradient hijau transparan.
8. **Garis kelembaban:** Gambar polyline hijau tebal 2.5px.
9. **Titik terakhir:** Lingkaran hijau solid bertepi putih untuk highlight data terkini.

**Re-render trigger:** Dipanggil dari `onMsg()`, `loadCfg()`, `saveConfig()`, dan `window.resize` event.

---

#### `loadCfg()` → `void`

**Tujuan:** Memuat konfigurasi dari endpoint `GET /api/config` dan mengisi form input.

**Cara kerja:** `fetch('/api/config')` → parse JSON → isi 10 field input form (termasuk threshold yang digunakan `drawChart()`). Panggil `drawChart()` untuk perbarui garis threshold di chart.

---

#### `saveConfig()` → `void`

**Tujuan:** Baca semua input form, kirim ke firmware via `POST /api/config`.

**Cara kerja:**
1. Baca semua input form ke object `body` (konversi angka dengan unary `+`).
2. Merge ke `cfg` lokal (update `dryThreshold`/`wetThreshold` agar chart langsung update).
3. `fetch('/api/config', {method:'POST', headers: {'Content-Type':'application/json'}, body: JSON.stringify(body)})`.
4. Parse response, tampilkan toast sukses/gagal.
5. Jika sukses, panggil `drawChart()`.

---

#### `g(id)` → `HTMLElement`

**Tujuan:** Shorthand `document.getElementById(id)`. Mengurangi verbosity kode UI.

---

#### `setPill(type, msg)` → `void`

**Tujuan:** Update badge status WebSocket di header.

**Parameter:**
- `type` — class CSS: `"ok"` (hijau) atau `"warn"` (kuning).
- `msg` — teks yang ditampilkan di badge.

---

#### `fmtUptime(s)` → `string`

**Tujuan:** Format angka detik menjadi string waktu yang mudah dibaca.

**Parameter:** `s` *(number)* — detik sejak boot.

**Return value:** String format `HH:MM:SS` (jika < 1 hari) atau `Xh HH:MM:SS` (jika >= 1 hari). Mengembalikan `"—"` jika `s` adalah null.

---

#### `pad(n)` → `string`

**Tujuan:** Zero-pad angka ke 2 digit untuk format waktu.

**Parameter:** `n` *(number)* — angka yang akan di-pad.

**Return value:** String 2 karakter (`pad(5)` → `"05"`, `pad(12)` → `"12"`).

---

#### `toast(type, msg)` → `void`

**Tujuan:** Tampilkan notifikasi toast sementara di pojok kanan atas halaman.

**Parameter:**
- `type` *(string)* — `"ok"` (hijau), `"err"` (merah), `"info"` (biru), atau `"warn"` (kuning).
- `msg` *(string)* — teks pesan.

**Siklus hidup toast (3.5 detik):**
1. Buat elemen `<div class="toast {type}">`.
2. Append ke `#toasts` container.
3. Double `requestAnimationFrame` untuk force reflow sebelum transisi CSS (teknik untuk trigger CSS transition pada element yang baru ditambahkan).
4. Tambahkan class `"show"` → CSS transition `opacity 0→1` + `translateX 16px→0`.
5. Setelah 3500 ms: hapus class `"show"` → fade out.
6. Setelah 350 ms tambahan: `.remove()` dari DOM.

---

## 7. State Machine Pompa

```
                     ┌──────────────────────────────────────────────────┐
                     │                                                  │
                     ▼                                                  │
              ┌─────────────┐                                           │
    boot ───► │    IDLE     │                                           │
              └──────┬──────┘                                           │
                     │ soil < DRY_THRESHOLD (30%)                       │
                     ▼                                                  │
              ┌─────────────┐                                           │
              │  WATERING   │                                           │
              └──────┬──────┘                                           │
                     │                                                  │
           ┌─────────┴──────────┐                                       │
           │                    │                                       │
           │ (soil > WET (55%)  │ elapsed > MAX_ON (30s)                │
           │  AND elapsed       │ [safety cutoff]                       │
           │  > MIN_ON (8s))    │                                       │
           ▼                    ▼                                       │
      ┌──────────┐         ┌──────────┐                                 │
      │ COOLDOWN │ ◄─────  │ COOLDOWN │                                 │
      └────┬─────┘  force  └──────────┘                                 │
           │   off                  ▲                                   │
           │ elapsed > COOLDOWN_MS  │                                   │
           │ (90s)                  │ manual duration expired           │
           └────────────────────────┼─────────────────────────────────►─┘
                                    │
              ┌─────────────┐       │
              │  MANUAL_ON  │ ──────┘
              └─────────────┘
                    ▲
                    │ forceOn(durationMs)
                    │ (dari web UI atau MQTT)
```

**Tabel transisi:**

| From | Kondisi | To |
|------|---------|-----|
| IDLE | `soil < 30%` | WATERING |
| WATERING | `soil > 55%` AND `elapsed >= 8s` | COOLDOWN |
| WATERING | `elapsed >= 30s` (safety) | COOLDOWN |
| COOLDOWN | `elapsed >= 90s` | IDLE |
| MANUAL_ON | `millis() >= manualEndMs` | COOLDOWN |
| Any | `forceOn(ms)` dipanggil | MANUAL_ON |
| Any | `forceOff()` dipanggil | COOLDOWN |

---

## 8. MQTT — Topik & Payload

### Topik Publish (Device → Broker)

| Topik | Payload | Retained | Interval |
|-------|---------|----------|----------|
| `smartplant/proto/soil` | `"65"` (string int) | Ya | 10 detik |
| `smartplant/proto/pump/state` | `"ON"` / `"OFF"` | Ya | 10 detik |
| `smartplant/proto/status` | JSON (lihat di bawah) | Ya | 10 detik |
| `smartplant/proto/availability` | `"online"` / `"offline"` (LWT) | Ya | Event |

**Payload `status` (JSON):**
```json
{
  "soil":     65,
  "pump":     false,
  "pump_state": 0,
  "uptime":   3600,
  "ip":       "192.168.1.55",
  "power":    "usb",
  "version":  "1.0.0"
}
```

`pump_state`: 0=IDLE, 1=WATERING, 2=COOLDOWN, 3=MANUAL

### Topik Subscribe (Broker → Device)

| Topik | Payload Diterima | Aksi |
|-------|-----------------|------|
| `smartplant/proto/pump/set` | `"ON"`, `"on"`, `"true"`, `"1"` | `pump.forceOn(5000)` |
| `smartplant/proto/pump/set` | `"OFF"`, `"off"`, `"false"`, `"0"` | `pump.forceOff()` |

### Home Assistant Auto-Discovery

Saat pertama kali connect ke broker, firmware otomatis mempublish konfigurasi discovery ke:
- `homeassistant/sensor/smartplant_proto_soil/config` — Soil Moisture sensor
- `homeassistant/switch/smartplant_proto_pump/config` — Pompa switch

Setelah ini, entitas langsung muncul di Home Assistant tanpa konfigurasi manual.

### LWT (Last Will Testament)

Saat connect, firmware mendaftarkan LWT:
- **Topic:** `smartplant/proto/availability`
- **Payload:** `"offline"` (dikirim broker otomatis saat device disconnect tidak normal)
- Saat connect normal: firmware publish `"online"` ke topik yang sama

---

## 9. HTTP API

Base URL: `http://<IP_DEVICE>/`

### GET `/`

Mengembalikan halaman web dashboard (`index.html` dari LittleFS).

### GET `/api/config`

Mengembalikan konfigurasi runtime saat ini dalam format JSON.

**Response:**
```json
{
  "mqttBroker":   "192.168.1.100",
  "mqttPort":     1883,
  "mqttUser":     "",
  "dryThreshold": 30,
  "wetThreshold": 55,
  "pumpMaxSec":   30,
  "cooldownSec":  90,
  "soilDryRaw":   850,
  "soilWetRaw":   380
}
```

> **Catatan:** `mqttPass` tidak dikembalikan di GET untuk keamanan.

### POST `/api/config`

Memperbarui satu atau lebih field konfigurasi dan menyimpan ke flash.

**Request body (JSON):**
```json
{
  "mqttBroker":   "192.168.1.10",
  "mqttPort":     1883,
  "mqttUser":     "user",
  "mqttPass":     "password",
  "dryThreshold": 25,
  "wetThreshold": 60,
  "pumpMaxSec":   45,
  "cooldownSec":  120,
  "soilDryRaw":   870,
  "soilWetRaw":   360
}
```

Field yang tidak disertakan tidak diubah. Semua field bersifat opsional.

**Response (sukses):**
```json
{"ok": true, "msg": "Config tersimpan. Restart untuk apply MQTT."}
```

**Response (error):**
```json
{"ok": false, "msg": "JSON invalid"}
```

> **Catatan:** Perubahan MQTT broker/port/kredensial memerlukan restart device untuk diterapkan.

---

## 10. WebSocket Protocol

**URL:** `ws://<IP_DEVICE>:81/`  
**Port:** 81  
**Format:** JSON text frames

### Server → Client (Push)

Dikirim saat: client baru connect, state berubah, atau heartbeat 30 detik.

```json
{
  "t":      "s",
  "soil":   65,
  "pump":   false,
  "state":  "IDLE",
  "uptime": 3600,
  "mqtt":   true,
  "ip":     "192.168.1.55",
  "ver":    "1.0.0"
}
```

| Field | Tipe | Keterangan |
|-------|------|------------|
| `t` | string | Type: selalu `"s"` (status) |
| `soil` | int | Kelembapan 0–100% |
| `pump` | bool | `true` jika relay aktif |
| `state` | string | "IDLE" / "WATERING" / "COOLDOWN" / "MANUAL" |
| `uptime` | int | Uptime dalam detik |
| `mqtt` | bool | `true` jika terhubung ke broker |
| `ip` | string | IP address device |
| `ver` | string | Versi firmware |

### Client → Server (Command)

**Nyalakan pompa manual:**
```json
{"cmd": "pump", "state": "on", "dur": 5000}
```

**Matikan pompa:**
```json
{"cmd": "pump", "state": "off"}
```

| Field | Tipe | Keterangan |
|-------|------|------------|
| `cmd` | string | Selalu `"pump"` |
| `state` | string | `"on"` atau `"off"` |
| `dur` | int | Durasi ms (hanya untuk `"on"`), di-clamp ke 1000–120000 |

---

## 11. Sistem Konfigurasi

### File `/config.json` di LittleFS

```json
{
  "mqttBroker":   "192.168.1.100",
  "mqttPort":     1883,
  "mqttUser":     "",
  "mqttPass":     "",
  "soilDryRaw":   850,
  "soilWetRaw":   380,
  "dryThreshold": 30,
  "wetThreshold": 55,
  "pumpMaxSec":   30,
  "cooldownSec":  90
}
```

### Cara Mengubah Konfigurasi

1. **Via Web Dashboard** — buka `http://<IP>/`, klik accordion "Pengaturan", isi form, klik Simpan
2. **Via HTTP API** — `POST /api/config` dengan body JSON
3. **Via Captive Portal** — saat first boot atau reset WiFi (MQTT params saja)
4. **Langsung di `config.h`** — untuk konstanta default (perlu recompile)

### Urutan Prioritas Konfigurasi

```
AppConfig defaults (hardcoded di struct)
    ↓ override jika file ada
loadConfig() dari /config.json
    ↓ override jika user isi di captive portal
WiFiManager params (MQTT fields saja)
    ↓ runtime update via
POST /api/config
```

---

## 12. Build & Flash

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI atau extension VS Code)
- Driver USB-Serial CH340 / CP2102 (NodeMCU v3 biasanya CH340)

### Langkah Build & Upload

```bash
# 1. Clone / buka direktori proyek
cd smart-plant-prototype

# 2. Build firmware
pio run

# 3. Upload filesystem (index.html → LittleFS)
#    WAJIB dilakukan minimal sekali sebelum atau setelah upload firmware
pio run -t uploadfs

# 4. Upload firmware ke ESP8266
pio run -t upload

# 5. Monitor Serial (115200 baud)
pio device monitor
```

> **Urutan:** Upload firmware dan filesystem bisa dilakukan dalam urutan apapun, tapi keduanya harus dilakukan. Jika hanya upload firmware tanpa filesystem, web server akan mengembalikan 404 untuk halaman utama.

### Upload Speed

Upload firmware menggunakan `921600` baud (dikonfigurasi di `platformio.ini`). Jika terjadi error, coba turunkan ke `460800` atau `230400` dengan mengedit:

```ini
upload_speed = 460800
```

### Ukuran Build (Referensi)

| Resource | Penggunaan | Kapasitas |
|----------|-----------|-----------|
| Flash (program) | ~42% | ~1 MB (dari 4 MB) |
| RAM (data) | ~44% | ~80 KB |

---

## 13. First Boot — Provisioning WiFi

Saat pertama kali dinyalakan (atau setelah reset WiFi):

1. OLED menampilkan "WiFi Setup Mode"
2. ESP8266 membuka hotspot: **`SmartPlant-Proto`** (password: `smartplant`)
3. Hubungkan smartphone/laptop ke hotspot tersebut
4. Buka browser, navigasi ke `http://192.168.4.1`
5. Klik **"Configure WiFi"**
6. Pilih SSID WiFi rumah, isi password
7. Isi parameter MQTT (opsional, bisa diubah nanti via web dashboard):
   - MQTT Broker IP
   - MQTT Port (default 1883)
   - MQTT User (kosongkan jika tidak ada auth)
   - MQTT Password (kosongkan jika tidak ada auth)
8. Klik **Save**
9. ESP8266 akan restart dan connect ke WiFi yang dipilih
10. OLED menampilkan IP address yang diberikan router

**Timeout:** Jika tidak ada yang mengakses captive portal dalam **3 menit**, ESP8266 restart otomatis dan coba lagi.

### Reset WiFi Tersimpan

Untuk menghapus kredensial WiFi dan memulai provisioning ulang, hapus data WiFiManager. Salah satu caranya: tambahkan `wm.resetSettings()` sementara di awal `setup()`, upload, lalu hapus lagi.

---

## 14. Kalibrasi Sensor

Sensor kelembapan kapasitif perlu dikalibrasi karena nilai ADC bervariasi antar sensor dan kondisi hardware.

### Langkah Kalibrasi

**Menentukan nilai DRY (udara kering):**
1. Jaga sensor tetap kering, tidak menyentuh apapun
2. Monitor Serial: `pio device monitor`
3. Catat nilai ADC yang muncul (contoh: 820–870)
4. Nilai ini adalah `soilDryRaw`

**Menentukan nilai WET (celup di air):**
1. Celupkan bagian sensor (bukan konektor) ke dalam air
2. Catat nilai ADC (contoh: 360–400)
3. Nilai ini adalah `soilWetRaw`

### Cara Update Nilai Kalibrasi

**Via Web Dashboard:**
1. Buka `http://<IP>/`
2. Klik accordion "Pengaturan"
3. Isi field "ADC Dry Raw" dan "ADC Wet Raw"
4. Klik Simpan

**Via `config.h` (default baru):**
```c
#define SOIL_RAW_DRY  870   // ganti sesuai hasil kalibrasi
#define SOIL_RAW_WET  365   // ganti sesuai hasil kalibrasi
```

### Verifikasi Kalibrasi

Setelah kalibrasi, cek di Serial monitor atau web dashboard:
- Sensor di udara → tampilkan ~0%
- Sensor di air → tampilkan ~100%
- Sensor di tanah kering → tampilkan 10–30%
- Sensor di tanah lembap → tampilkan 50–80%

---

## 15. Integrasi Home Assistant

### Auto-Discovery (Otomatis)

Firmware otomatis mendaftarkan dua entitas ke Home Assistant saat pertama kali connect ke MQTT broker. Tidak diperlukan konfigurasi YAML.

**Syarat:**
- Home Assistant dengan add-on MQTT Broker (Mosquitto) aktif
- MQTT Integration sudah dikonfigurasi di HA
- Device terhubung ke broker yang sama

**Entitas yang dibuat:**
1. `sensor.smart_plant_prototype_soil_moisture` — Kelembapan tanah (%)
2. `switch.smart_plant_prototype_pompa_air` — Kontrol pompa ON/OFF

### Konfigurasi Manual YAML (Alternatif)

Jika auto-discovery tidak bekerja, tambahkan ke `configuration.yaml`:

```yaml
mqtt:
  sensor:
    - name: "Smart Plant Soil"
      state_topic: "smartplant/proto/soil"
      unit_of_measurement: "%"
      device_class: moisture
      availability_topic: "smartplant/proto/availability"
      payload_available: "online"
      payload_not_available: "offline"

  switch:
    - name: "Smart Plant Pompa"
      state_topic: "smartplant/proto/pump/state"
      command_topic: "smartplant/proto/pump/set"
      payload_on: "ON"
      payload_off: "OFF"
      availability_topic: "smartplant/proto/availability"
      payload_available: "online"
      payload_not_available: "offline"
```

### Automation Contoh

```yaml
automation:
  - alias: "Smart Plant - Notifikasi Tanah Kering"
    trigger:
      - platform: numeric_state
        entity_id: sensor.smart_plant_prototype_soil_moisture
        below: 25
    action:
      - service: notify.mobile_app
        data:
          message: "Tanaman perlu disiram! Kelembapan {{ states('sensor.smart_plant_prototype_soil_moisture') }}%"
```

---

## 16. Web Dashboard

Dashboard dapat diakses di `http://<IP_DEVICE>/` menggunakan browser apapun di jaringan yang sama.

### Fitur

| Fitur | Keterangan |
|-------|------------|
| Gauge kelembapan real-time | SVG gauge animasi, warna berubah sesuai level |
| Grafik history | Chart 20 titik data terakhir dengan garis threshold |
| Status pompa | Badge warna dengan animasi pulse saat aktif |
| Kontrol pompa manual | Tombol 5 / 10 / 15 / 20 detik |
| Status koneksi | WiFi dan MQTT (termasuk broker address) |
| Informasi sistem | IP, uptime, versi firmware |
| Pengaturan | Form edit semua parameter config, lazy-loaded |
| Real-time update | WebSocket push tanpa reload/polling |

### Update Real-Time

Dashboard menggunakan WebSocket (port 81) untuk menerima update dari server:
- Update dikirim **hanya saat state berubah** (bukan polling 3 detik)
- Heartbeat setiap 30 detik sebagai keepalive
- Reconnect otomatis dengan exponential backoff (1s → 2s → 4s → ... → 16s max)

### Tema

Dashboard menggunakan tema **"Pagi Kebun"** — palet warna terang yang menggambarkan kesejukan taman:

| Variabel | Warna | Penggunaan |
|----------|-------|------------|
| `--leaf` | `#22a852` | Elemen utama, progress, koneksi OK |
| `--sun` | `#c87e08` | Aksen header, judul kartu |
| `--flower` | `#c44070` | Aksen dekoratif |
| `--sky` | `#1870b4` | Link, info teknis |
| `--bg` | `#eef7e6` | Background halaman |

---

## 17. Troubleshooting

### MQTT tidak bisa connect (`rc=-2`)

**Gejala:** Serial menampilkan `[MQTT] Connecting... FAIL rc=-2`

**Penyebab:** Setelah disconnect, `WiFiClient` di ESP8266 bisa tersangkut di state `CLOSE_WAIT`/`FIN_WAIT`. Socket lama tidak tertutup otomatis, sehingga upaya connect baru gagal.

**Solusi (sudah diimplementasikan):** `_wifi.stop() + yield()` sebelum `_client.connect()` di fungsi `_reconnect()`. Jika masih terjadi, cek:
- Apakah broker MQTT aktif dan bisa diakses dari jaringan yang sama
- Apakah IP broker di konfigurasi sudah benar
- Cek port, firewall, atau autentikasi jika diaktifkan

### LittleFS Error saat boot

**Gejala:** Serial menampilkan `[FS] FAILED — format dulu: pio run -t uploadfs`

**Solusi:**
```bash
pio run -t uploadfs
```

Pastikan `data/index.html` ada di direktori proyek sebelum menjalankan perintah ini.

### Web dashboard tidak bisa dibuka (404)

**Penyebab:** LittleFS belum diupload atau file `index.html` tidak ada di flash.

**Solusi:** Jalankan `pio run -t uploadfs` lalu restart device.

### WiFi tidak bisa connect setelah provisioning

**Gejala:** Device terus masuk ke AP mode setiap boot.

**Kemungkinan penyebab:**
- Password WiFi salah saat provisioning
- Router tidak mengirim DHCP response
- SSID WiFi berubah

**Solusi:** Reset settings WiFiManager dan provisioning ulang.

### OLED tidak menyala

**Kemungkinan penyebab:**
- Koneksi I2C longgar (SDA/SCL)
- Alamat I2C salah (default 0x3C, beberapa modul pakai 0x3D)
- Power tidak mencukupi

**Cek:** Firmware tetap berjalan meski OLED gagal (non-fatal). Lihat Serial monitor untuk memastikan firmware berjalan normal.

### Sensor selalu menampilkan 0% atau 100%

**Penyebab:** Nilai kalibrasi `soilDryRaw` dan `soilWetRaw` tidak sesuai dengan sensor yang digunakan.

**Solusi:** Lakukan kalibrasi ulang (lihat bagian [Kalibrasi Sensor](#14-kalibrasi-sensor)).

### Pompa tidak berhenti (stuck WATERING)

**Proteksi yang ada:** Safety cutoff `PUMP_MAX_ON_MS` (30 detik). Jika tanah tidak pernah mencapai `wetThreshold` setelah 30 detik, pompa otomatis mati dan masuk COOLDOWN.

**Jika tetap bermasalah:** Gunakan `forceOff()` via web dashboard (tombol OFF) atau MQTT `pump/set` → `"OFF"`.

---

## 18. Pengembangan Lanjutan

### Versi 2.0 (Roadmap)

- [ ] Migrasi ke **ESP32** untuk dual-core, lebih banyak GPIO, dan ADC yang lebih akurat
- [ ] Integrasi **RTC (DS3231)** untuk jadwal penyiraman berbasis waktu
- [ ] Multiple sensor zones (beberapa pot, satu controller)
- [ ] Data logging ke SD card atau InfluxDB via MQTT
- [ ] OTA (Over-the-Air) firmware update
- [ ] Battery + solar panel mode dengan deep sleep
- [ ] Sensor suhu dan kelembapan udara (DHT22/SHT31)
- [ ] Dashboard lebih canggih dengan Grafana + InfluxDB

### Perbaikan Minor

- [ ] Tambahkan endpoint `/api/restart` untuk restart via HTTP
- [ ] Tambahkan endpoint `/api/pump` untuk kontrol pompa via REST (bukan hanya WS)
- [ ] WebSocket authentication (token sederhana)
- [ ] Notifikasi push via webhook saat tanah sangat kering

---

## Lisensi

Proyek ini dibuat untuk keperluan prototipe pribadi / edukasi.

---

*Smart Plant Prototype v1.0.0 — ESP8266 NodeMCU v3*
