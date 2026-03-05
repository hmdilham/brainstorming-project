# Modul Pertemuan 12 — IOT Dasar
# Library Populer ESP32 & Pengenalan ESPHome

---

## 1. Maksud dan Tujuan Materi

### Maksud
Library/pustaka yang tepat mempercepat pengembangan secara drastis. Pertemuan ini mengenalkan tiga library esensial: **WiFiManager** (konfigurasi WiFi tanpa hardcode), **ArduinoJson** (parse/buat JSON), dan **Preferences** (penyimpanan data persisten). Ditambah pengenalan **ESPHome** — framework deklaratif yang memungkinkan konfigurasi ESP32 dengan YAML tanpa menulis kode C++.

### Tujuan Pembelajaran
1. **Menggunakan** WiFiManager untuk konfigurasi WiFi via captive portal.
2. **Memparse** dan membuat JSON menggunakan ArduinoJson.
3. **Menyimpan** data ke NVS (Non-Volatile Storage) dengan library Preferences.
4. **Menjelaskan** konsep ESPHome dan perbandingannya dengan PlatformIO.
5. **Menginstal** ESPHome dan membuat konfigurasi YAML pertama.

---

## 2. Teori Materi

### 2.1 WiFiManager (by tzapu)

WiFiManager menghilangkan kebutuhan hardcode SSID/password di kode. Jika ESP32 tidak bisa terhubung ke WiFi yang tersimpan, otomatis masuk mode AP (hotspot) dan membuka captive portal di browser untuk user memilih jaringan WiFi.

```
  Alur WiFiManager:
  
  Boot → Coba WiFi tersimpan → Berhasil? → Lanjut program
                                   │
                                   ❌ Gagal
                                   │
                              Buat AP hotspot
                              "ESP32-Setup"
                                   │
                              User konek ke AP
                              Buka 192.168.4.1
                                   │
                              Pilih WiFi + isi password
                                   │
                              WiFi tersimpan → Reboot → Terhubung ✅
```

### 2.2 ArduinoJson (by Benoît Blanchon)

ArduinoJson adalah library de-facto untuk parsing dan pembuatan data JSON di mikrokontroler. JSON adalah format data standar di web dan IoT.

```cpp
// Parsing JSON
const char* json = "{\"suhu\":28.5,\"lembap\":65}";
JsonDocument doc;
deserializeJson(doc, json);
float suhu = doc["suhu"];    // 28.5
int lembap = doc["lembap"];  // 65

// Membuat JSON
JsonDocument out;
out["device"] = "ESP32";
out["suhu"] = 28.5;
String payload;
serializeJson(out, payload);
// payload = {"device":"ESP32","suhu":28.5}
```

### 2.3 Preferences (NVS — Non-Volatile Storage)

Preferences menyimpan data ke partisi NVS di flash ESP32. Data bertahan meskipun ESP32 di-reset atau mati.

```cpp
#include <Preferences.h>
Preferences prefs;

prefs.begin("config", false);        // Namespace "config", RW mode
prefs.putString("ssid", "MyWiFi");   // Simpan string
prefs.putInt("counter", 42);         // Simpan integer
String ssid = prefs.getString("ssid", "default");  // Baca (+ default)
int cnt = prefs.getInt("counter", 0);
prefs.end();
```

### 2.4 Pengenalan ESPHome

**ESPHome** adalah framework open-source yang memungkinkan pengguna mengonfigurasi perangkat ESP32/ESP8266 menggunakan **file YAML** sederhana — tanpa perlu menulis kode C++.

**Perbandingan PlatformIO vs ESPHome:**

| Aspek | PlatformIO (C++) | ESPHome (YAML) |
|---|---|---|
| Kode | C++ (`main.cpp`) | YAML deklaratif |
| Kemampuan | Tak terbatas | Terbatas pada komponen yang tersedia |
| Belajar | Perlu pelajari C++ | **Sangat mudah — hanya YAML** |
| Custom logic | Ya (penuh) | Via `lambda` C++ (terbatas) |
| Integrasi HA | Manual (MQTT) | **Native (otomatis!)** |
| OTA Update | Perlu kode sendiri | **Built-in** |
| Dashboard | Perlu buat sendiri | **Gratis via Home Assistant** |
| Ideal untuk | Proyek custom, complex | **Smart home, automasi sederhana** |

**Contoh Konfigurasi ESPHome:**
```yaml
esphome:
  name: sensor-ruangan
  platform: ESP32
  board: esp32dev

wifi:
  ssid: "NamaWiFi"
  password: "PasswordWiFi"

sensor:
  - platform: dht
    pin: GPIO4
    model: DHT22
    temperature:
      name: "Suhu Ruangan"
    humidity:
      name: "Kelembaban Ruangan"
    update_interval: 5s

switch:
  - platform: gpio
    pin: GPIO2
    name: "LED Ruangan"
```

> File YAML di atas **menggantikan** ~100 baris kode C++ dan otomatis terintegrasi dengan Home Assistant!

---

## 3. Panduan Praktikum

### Praktikum 3.1 — WiFiManager + Preferences Counter + OLED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    https://github.com/tzapu/WiFiManager.git
    bblanchon/ArduinoJson@^7.0.0
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : WiFiManager + Preferences + OLED
 * DESKRIPSI:
 *   - WiFiManager: captive portal untuk konfigurasi WiFi
 *   - Preferences: counter boot persisten
 *   - OLED: menampilkan status WiFi, IP, counter, uptime
 *
 * CARA PAKAI:
 *   1. Upload firmware
 *   2. ESP32 membuat hotspot "ESP32-Setup"
 *   3. Hubungkan HP ke hotspot → browser otomatis buka → pilih WiFi
 *   4. Setelah terhubung, OLED menampilkan info
 */

#include <Arduino.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Preferences prefs;

int bootCount = 0;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }

  // Boot counter
  prefs.begin("app", false);
  bootCount = prefs.getInt("boots", 0) + 1;
  prefs.putInt("boots", bootCount);
  prefs.end();
  Serial.printf("Boot ke-%d\n", bootCount);

  // OLED: menunggu WiFi
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("WiFiManager");
  display.setCursor(0, 15);
  display.println("Jika belum ada WiFi:");
  display.setCursor(0, 25);
  display.println("Konek ke hotspot:");
  display.setCursor(0, 40);
  display.println("  SSID: ESP32-Setup");
  display.setCursor(0, 50);
  display.printf("  Boot: #%d", bootCount);
  display.display();

  // WiFiManager
  WiFiManager wm;
  // wm.resetSettings(); // Uncomment untuk reset WiFi tersimpan

  bool connected = wm.autoConnect("ESP32-Setup", "12345678");

  if (!connected) {
    Serial.println("WiFi gagal! Restart...");
    delay(3000);
    ESP.restart();
  }

  Serial.printf("WiFi OK! IP: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("WiFi Connected!");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("SSID: %s", WiFi.SSID().c_str());
  display.setCursor(0, 23);
  display.printf("IP  : %s", WiFi.localIP().toString().c_str());
  display.setCursor(0, 33);
  display.printf("RSSI: %d dBm", WiFi.RSSI());
  display.setCursor(0, 43);
  display.printf("Boot: #%d", bootCount);
  display.setCursor(0, 53);
  display.printf("Up  : %lu detik", millis() / 1000);

  display.display();
  delay(1000);
}
```

---

### Praktikum 3.2 — ArduinoJson: Parse dan Buat JSON

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : ArduinoJson Demo
 * DESKRIPSI: Demonstrasi parsing JSON string dan membuat JSON payload.
 *            Output di Serial Monitor dan OLED.
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }
  delay(1000);

  // === PARSING JSON ===
  Serial.println("=== Parsing JSON ===");
  const char* jsonInput = "{\"device\":\"ESP32\",\"suhu\":28.5,\"lembap\":65,\"status\":\"OK\"}";
  Serial.printf("Input: %s\n\n", jsonInput);

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, jsonInput);

  if (err) {
    Serial.printf("Parse error: %s\n", err.c_str());
    return;
  }

  const char* device = doc["device"];
  float suhu = doc["suhu"];
  int lembap = doc["lembap"];
  const char* status = doc["status"];

  Serial.printf("Device : %s\n", device);
  Serial.printf("Suhu   : %.1f°C\n", suhu);
  Serial.printf("Lembap : %d%%\n", lembap);
  Serial.printf("Status : %s\n\n", status);

  // === MEMBUAT JSON ===
  Serial.println("=== Membuat JSON ===");
  JsonDocument out;
  out["device"] = "ESP32-Sensor";
  out["data"]["suhu"] = 29.3;
  out["data"]["lembap"] = 72;
  out["uptime_ms"] = millis();

  String payload;
  serializeJsonPretty(out, payload);
  Serial.println(payload);

  // === OLED ===
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ArduinoJson Demo");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 13);
  display.printf("Device: %s", device);
  display.setCursor(0, 23);
  display.printf("Suhu  : %.1f C", suhu);
  display.setCursor(0, 33);
  display.printf("Lembap: %d %%", lembap);
  display.setCursor(0, 43);
  display.printf("Status: %s", status);
  display.setCursor(0, 55);
  display.println("JSON Parsed OK!");
  display.display();
}

void loop() {
  delay(10000);
}
```

---

### Praktikum 3.3 — Instalasi ESPHome (Pengenalan)

**Langkah-langkah:**

1. **Instal ESPHome via pip:**
   ```bash
   pip install esphome
   ```

2. **Buat file konfigurasi `sensor-ruangan.yaml`:**
   ```yaml
   esphome:
     name: sensor-ruangan

   esp32:
     board: esp32dev
     framework:
       type: arduino

   logger:

   wifi:
     ssid: "NamaWiFi"
     password: "PasswordAnda"
     ap:
       ssid: "sensor-ruangan-fallback"
       password: "fallback123"

   captive_portal:

   api:
     encryption:
       key: "your-encryption-key"

   ota:
     - platform: esphome
       password: "ota-password"

   sensor:
     - platform: dht
       pin: GPIO4
       model: DHT22
       temperature:
         name: "Suhu Ruangan"
       humidity:
         name: "Kelembaban Ruangan"
       update_interval: 5s

   switch:
     - platform: gpio
       pin: GPIO2
       name: "LED Ruangan"
   ```

3. **Compile dan upload:**
   ```bash
   esphome run sensor-ruangan.yaml
   ```

4. **Dashboard ESPHome:**
   ```bash
   esphome dashboard ./
   # Buka browser: http://localhost:6052
   ```

> **Di pertemuan 13**, ESPHome akan diintegrasikan dengan Home Assistant untuk dashboard dan automasi lengkap.

---

## 4. Referensi

1. **WiFiManager.** [https://github.com/tzapu/WiFiManager](https://github.com/tzapu/WiFiManager)
2. **ArduinoJson.** [https://arduinojson.org/](https://arduinojson.org/)
3. **Espressif.** *Preferences (NVS).* [https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html)
4. **ESPHome.** *Getting Started.* [https://esphome.io/guides/getting_started_command_line.html](https://esphome.io/guides/getting_started_command_line.html)
