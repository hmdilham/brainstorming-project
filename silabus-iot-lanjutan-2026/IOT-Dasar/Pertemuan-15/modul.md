# Modul Pertemuan 15 — IOT Dasar
# Integrasi Sistem & Persiapan Proyek Akhir

---

## 1. Maksud dan Tujuan Materi

### Maksud
Menggabungkan seluruh konsep yang telah dipelajari (GPIO, sensor, aktuator, WiFi, BLE, Web Server, ESPHome/Home Assistant) menjadi satu sistem IoT yang utuh. Mahasiswa mempelajari arsitektur sistem, teknik non-blocking code, troubleshooting, dan mempersiapkan desain proyek akhir.

### Tujuan Pembelajaran
1. **Mendesain** arsitektur sistem IoT dari sensor hingga dashboard.
2. **Mengimplementasikan** multi-tasking non-blocking menggunakan `millis()`.
3. **Menerapkan** best practices: modularisasi, error handling, reconnect otomatis.
4. **Memvalidasi** desain proyek akhir dengan dosen.

---

## 2. Teori Materi

### 2.1 Arsitektur Sistem IoT Terpadu

```
  ┌── Sensor Layer ─────────────────────────────────────┐
  │  DHT22 │ HC-SR04 │ LDR │ BMP280 │ PIR              │
  └────────────────┬────────────────────────────────────┘
                   │  GPIO / I2C / SPI / ADC
  ┌────────────────▼────────────────────────────────────┐
  │  ESP32 — Firmware (PlatformIO / ESPHome)            │
  │  ┌──────────────┐  ┌────────────┐  ┌─────────────┐ │
  │  │ Baca Sensor  │  │ Kontrol    │  │ Komunikasi  │ │
  │  │ (non-block)  │  │ Aktuator   │  │ WiFi/BLE    │ │
  │  └──────────────┘  └────────────┘  └──────┬──────┘ │
  └───────────────────────────────────────────┼────────┘
                                              │  WiFi
  ┌───────────────────────────────────────────▼────────┐
  │  Dashboard / Cloud                                  │
  │  • Home Assistant  • Blynk  • Web Server Lokal     │
  │  • MQTT Broker     • REST API                       │
  └─────────────────────────────────────────────────────┘
```

### 2.2 Non-Blocking Code dengan `millis()`

```cpp
// BURUK — delay() memblok seluruh program
void loop() {
  bacaSensor();   // 100ms
  delay(2000);    // BLOK 2 detik!! Tidak bisa baca tombol!
  updateOLED();
  delay(1000);    // BLOK lagi!!
}

// BAIK — millis() non-blocking
unsigned long lastSensor = 0;
unsigned long lastOLED   = 0;

void loop() {
  unsigned long now = millis();

  if (now - lastSensor >= 2000) {   // Setiap 2 detik
    lastSensor = now;
    bacaSensor();
  }

  if (now - lastOLED >= 1000) {     // Setiap 1 detik
    lastOLED = now;
    updateOLED();
  }

  cekTombol();  // Selalu responsif! Tidak terblok.
}
```

### 2.3 Troubleshooting Umum

| Masalah | Penyebab | Solusi |
|---|---|---|
| ESP32 reboot terus | Watchdog timeout / stack overflow | Kurangi delay(), cek rekursi |
| WiFi disconnect | Router jauh / interferensi | Auto-reconnect, cek RSSI |
| Sensor NaN/error | Kabel longgar / timing | Cek wiring, tambah pull-up |
| Upload gagal | Port salah / boot mode | Tekan BOOT saat upload |
| OLED tidak tampil | Alamat I2C salah | Jalankan I2C Scanner |

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Multi-Task Non-Blocking + OLED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/DHT sensor library@^1.4.6
    adafruit/Adafruit Unified Sensor@^1.1.14
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Multi-Task Non-Blocking System
 * DESKRIPSI: Menjalankan 4 task secara simultan tanpa delay():
 *   Task 1: Baca DHT22 setiap 2 detik
 *   Task 2: Baca push button setiap 50ms (responsive)
 *   Task 3: Update OLED setiap 500ms
 *   Task 4: LED blink pattern setiap 1 detik
 *   Task 5: Serial report setiap 5 detik
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(4, DHT22);

const uint8_t PIN_BTN = 15;
const uint8_t PIN_LED = 2;

// Data
float suhu = 0, hum = 0;
bool ledState = false;
int btnCount = 0;

// Timers
unsigned long tSensor = 0, tOLED = 0, tLED = 0, tSerial = 0;
unsigned long tBtn = 0;
bool lastBtnStable = HIGH, lastBtnRaw = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
  }
  Serial.println("Multi-Task Non-Blocking System");
}

void loop() {
  unsigned long now = millis();

  // === TASK 1: Baca Sensor (2000ms) ===
  if (now - tSensor >= 2000) {
    tSensor = now;
    float s = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(s)) { suhu = s; hum = h; }
  }

  // === TASK 2: Debounce Button (50ms) ===
  bool raw = digitalRead(PIN_BTN);
  if (raw != lastBtnRaw) tBtn = now;
  lastBtnRaw = raw;
  if ((now - tBtn) > 50 && raw != lastBtnStable) {
    lastBtnStable = raw;
    if (raw == LOW) { btnCount++; ledState = !ledState; }
  }
  digitalWrite(PIN_LED, ledState);

  // === TASK 3: Update OLED (500ms) ===
  if (now - tOLED >= 500) {
    tOLED = now;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Non-Blocking Demo");
    display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
    display.setCursor(0, 13);
    display.printf("Suhu : %.1f C", suhu);
    display.setCursor(0, 23);
    display.printf("Hum  : %.1f %%", hum);
    display.setCursor(0, 33);
    display.printf("LED  : %s", ledState ? "ON" : "OFF");
    display.setCursor(0, 43);
    display.printf("Press: %d", btnCount);
    display.setCursor(0, 55);
    display.printf("Up: %lu s", now / 1000);
    display.display();
  }

  // === TASK 4: Serial Report (5000ms) ===
  if (now - tSerial >= 5000) {
    tSerial = now;
    Serial.printf("[%lus] T=%.1f H=%.1f LED=%s Btn=%d\n",
                  now/1000, suhu, hum, ledState?"ON":"OFF", btnCount);
  }
}
```

---

### Praktikum 3.2 — Konsultasi Desain Proyek Akhir

Setiap mahasiswa/kelompok mempresentasikan desain proyek:
1. Nama proyek dan masalah yang dipecahkan
2. Diagram arsitektur (sensor → ESP32 → dashboard)
3. Daftar komponen hardware
4. Platform (Web Server / Blynk / Home Assistant)
5. Estimasi kesulitan dan timeline

---

## 4. Referensi

1. **Arduino.** *millis() Reference.* [https://www.arduino.cc/reference/en/language/functions/time/millis/](https://www.arduino.cc/reference/en/language/functions/time/millis/)
2. **Random Nerd Tutorials.** *ESP32 Multitasking.* [https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/](https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/)
