# Modul Pertemuan 6 — IOT Dasar
# Protokol Komunikasi — I2C (ESP32 ↔ Sensor/Display)

---

## 1. Maksud dan Tujuan Materi

### Maksud
I2C (Inter-Integrated Circuit) adalah protokol komunikasi serial **sinkron** yang hanya membutuhkan 2 kabel (SDA + SCL) untuk menghubungkan banyak perangkat. I2C sangat populer di IoT karena efisiensi pinnya — satu bus I2C bisa menghubungkan hingga 127 perangkat berbeda (sensor, display, RTC, EEPROM) secara bersamaan.

### Tujuan Pembelajaran
1. **Menjelaskan** arsitektur I2C: Master-Slave, SDA, SCL, alamat 7-bit.
2. **Mengimplementasikan** I2C Scanner untuk menemukan perangkat di bus.
3. **Membaca** data suhu dan kelembaban dari sensor DHT22.
4. **Menampilkan** data pada OLED SSD1306 128×64 menggunakan library Adafruit.
5. **Menghubungkan** beberapa perangkat I2C pada satu bus secara bersamaan.

---

## 2. Teori Materi

### 2.1 Arsitektur I2C

```
              I2C Bus (2 kabel)
  ─────────────┬──────────┬──────────┬──── SDA (Serial Data)
  ─────────────┬──────────┬──────────┬──── SCL (Serial Clock)
               │          │          │
        ┌──────┴────┐ ┌───┴───┐ ┌───┴───┐
        │  MASTER   │ │ SLAVE │ │ SLAVE │
        │  (ESP32)  │ │ OLED  │ │ DHT22 │
        │           │ │ 0x3C  │ │ 0x40  │
        └───────────┘ └───────┘ └───────┘
```

**Karakteristik I2C:**

| Parameter | Nilai |
|---|---|
| Jumlah kabel | 2 (SDA + SCL) + GND |
| Kecepatan | Standard: 100 kbps, Fast: 400 kbps |
| Jarak max (praktis) | ~1 meter (butuh pull-up yang tepat) |
| Max perangkat | 127 (7-bit address) |
| Komunikasi | Half-duplex (satu arah per waktu) |
| Pin ESP32 default | SDA = GPIO 21, SCL = GPIO 22 |

**Proses Komunikasi I2C:**
1. Master mengirim **Start Condition** (SDA turun saat SCL tinggi)
2. Master mengirim **alamat slave** (7 bit) + bit R/W (0=write, 1=read)
3. Slave merespons dengan **ACK** (acknowledge)
4. Data ditransfer (byte demi byte, setiap byte diakhiri ACK)
5. Master mengirim **Stop Condition** (SDA naik saat SCL tinggi)

### 2.2 Pull-Up Resistor

I2C membutuhkan resistor pull-up pada kedua line (SDA dan SCL). Banyak modul breakout sudah menyertakan pull-up on-board.

```
  3.3V ──[4.7kΩ]──┬── SDA ──── OLED SDA / Sensor SDA
                   │
  3.3V ──[4.7kΩ]──┬── SCL ──── OLED SCL / Sensor SCL
```

> **Catatan:** Kebanyakan modul breakout (OLED, BMP280, dll.) sudah memiliki pull-up on-board. Jika hanya 1-2 perangkat, pull-up tambahan biasanya tidak diperlukan.

### 2.3 Alamat I2C Perangkat Umum

| Perangkat | Alamat (hex) | Fungsi |
|---|---|---|
| OLED SSD1306 | `0x3C` atau `0x3D` | Display 128×64 |
| BMP280 / BME280 | `0x76` atau `0x77` | Sensor suhu/tekanan/kelembaban |
| MPU6050 | `0x68` atau `0x69` | Akselerometer + Gyroscope |
| ADS1115 | `0x48` – `0x4B` | ADC eksternal 16-bit |
| PCF8574 | `0x20` – `0x27` | I/O Expander 8-bit |
| DS3231 | `0x68` | RTC (Real-Time Clock) |

### 2.4 Sensor DHT22

DHT22 adalah sensor suhu dan kelembaban yang populer dan murah.

| Parameter | Nilai |
|---|---|
| Range Suhu | -40°C ~ 80°C (±0.5°C akurasi) |
| Range Kelembaban | 0–100% RH (±2% akurasi) |
| Sampling Rate | Maksimal 1 reading per 2 detik |
| Tegangan | 3.3V – 5V |
| Protokol | One-wire proprietary (BUKAN I2C) |

> **Catatan:** DHT22 sebenarnya bukan perangkat I2C murni — menggunakan protokol one-wire proprietary. Namun sering dipasangkan dengan perangkat I2C (OLED) pada breadboard yang sama.

**Skema Koneksi DHT22:**
```
  DHT22 Pin 1 (VCC)  → 3.3V
  DHT22 Pin 2 (DATA) → GPIO 4 + [10kΩ pull-up ke 3.3V]
  DHT22 Pin 3 (NC)   → Tidak dihubungkan
  DHT22 Pin 4 (GND)  → GND
```

### 2.5 OLED SSD1306 128×64

Display OLED monokom (putih/biru) yang sangat populer untuk proyek IoT.

| Parameter | Nilai |
|---|---|
| Resolusi | 128 × 64 piksel |
| Interface | I2C (2 kabel) |
| Alamat Default | `0x3C` |
| Tegangan | 3.3V – 5V |
| Library | Adafruit SSD1306 + Adafruit GFX |

---

## 3. Panduan Praktikum

### Praktikum 3.1 — I2C Scanner

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : I2C Scanner — Deteksi Perangkat di Bus
 * DESKRIPSI: Memindai seluruh alamat I2C (0x01-0x7F) dan menampilkan
 *            perangkat yang terdeteksi. Jalankan ini pertama kali untuk
 *            memverifikasi koneksi hardware.
 *
 * RANGKAIAN:
 *   GPIO 21 → SDA (ke semua perangkat I2C)
 *   GPIO 22 → SCL (ke semua perangkat I2C)
 */

#include <Arduino.h>
#include <Wire.h>

void scanI2C() {
  int found = 0;

  Serial.println("Memindai bus I2C...\n");
  Serial.println("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

  for (uint8_t row = 0; row < 8; row++) {
    Serial.printf("%02X: ", row * 16);
    for (uint8_t col = 0; col < 16; col++) {
      uint8_t addr = row * 16 + col;
      if (addr < 0x03 || addr > 0x77) {
        Serial.print("   ");
        continue;
      }

      Wire.beginTransmission(addr);
      uint8_t err = Wire.endTransmission();

      if (err == 0) {
        Serial.printf("%02X ", addr);
        found++;
      } else {
        Serial.print("-- ");
      }
    }
    Serial.println();
  }

  Serial.printf("\n%d perangkat ditemukan.\n", found);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA=21, SCL=22
  delay(1000);

  Serial.println("═══════════════════════════");
  Serial.println("  ESP32 I2C Scanner");
  Serial.println("═══════════════════════════");
  scanI2C();
}

void loop() {
  delay(5000);
  scanI2C();
}
```

---

### Praktikum 3.2 — DHT22 + OLED Display Real-Time

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

**Skema Rangkaian:**
```
  DHT22 DATA → GPIO 4 (+ 10kΩ pull-up ke 3.3V)
  DHT22 VCC  → 3.3V
  DHT22 GND  → GND

  OLED SDA   → GPIO 21
  OLED SCL   → GPIO 22
  OLED VCC   → 3.3V
  OLED GND   → GND
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : DHT22 + OLED — Monitor Suhu & Kelembaban
 * DESKRIPSI: Membaca sensor DHT22 setiap 2 detik dan menampilkan
 *            data suhu, kelembaban, heat index, min/max, dan grafik
 *            bar pada OLED display. Data juga dikirim ke Serial.
 *
 * RANGKAIAN:
 *   GPIO 4  → DHT22 DATA (+ 10kΩ pull-up ke 3.3V)
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// DHT22
#define DHT_PIN  4
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Statistik
float minTemp = 999, maxTemp = -999;
float minHum  = 999, maxHum  = -999;
int readCount = 0;

unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 2000; // DHT22 min interval: 2 detik

void setup() {
  Serial.begin(115200);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
    while (true) delay(1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.println("DHT22 + OLED");
  display.setCursor(25, 40);
  display.println("Starting...");
  display.display();
  delay(2000);

  Serial.println("DHT22 + OLED Monitor");
  Serial.println("Suhu(°C) | Kelembaban(%) | HeatIndex");
}

void loop() {
  if (millis() - lastRead < READ_INTERVAL) return;
  lastRead = millis();

  float suhu  = dht.readTemperature();
  float hum   = dht.readHumidity();

  if (isnan(suhu) || isnan(hum)) {
    Serial.println("[ERR] Gagal baca DHT22!");
    display.clearDisplay();
    display.setCursor(10, 25);
    display.setTextSize(2);
    display.println("SENSOR");
    display.setCursor(10, 45);
    display.println("ERROR!");
    display.display();
    return;
  }

  readCount++;
  float heatIndex = dht.computeHeatIndex(suhu, hum, false);

  // Update min/max
  if (suhu < minTemp) minTemp = suhu;
  if (suhu > maxTemp) maxTemp = suhu;
  if (hum < minHum)   minHum  = hum;
  if (hum > maxHum)   maxHum  = hum;

  // Serial output
  Serial.printf("[#%d] %.1f°C | %.1f%% | HI: %.1f°C | Min: %.1f Max: %.1f\n",
                readCount, suhu, hum, heatIndex, minTemp, maxTemp);

  // === OLED Display ===
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("Cuaca Lokal  #%d", readCount);
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Suhu (besar)
  display.setTextSize(2);
  display.setCursor(0, 13);
  display.printf("%.1f", suhu);
  display.setTextSize(1);
  display.print(" C");

  // Kelembaban
  display.setTextSize(2);
  display.setCursor(75, 13);
  display.printf("%.0f", hum);
  display.setTextSize(1);
  display.print(" %");

  // Bar suhu (0-50°C range)
  display.setTextSize(1);
  display.setCursor(0, 33);
  display.print("T:");
  display.drawRect(14, 33, 114, 7, SSD1306_WHITE);
  int tBar = constrain(map((int)(suhu*10), 0, 500, 0, 112), 0, 112);
  display.fillRect(15, 34, tBar, 5, SSD1306_WHITE);

  // Bar kelembaban (0-100%)
  display.setCursor(0, 42);
  display.print("H:");
  display.drawRect(14, 42, 114, 7, SSD1306_WHITE);
  int hBar = constrain(map((int)hum, 0, 100, 0, 112), 0, 112);
  display.fillRect(15, 43, hBar, 5, SSD1306_WHITE);

  // Min/Max & Heat Index
  display.setCursor(0, 53);
  display.printf("%.0f~%.0fC  HI:%.1fC", minTemp, maxTemp, heatIndex);

  display.display();
}
```

---

### Praktikum 3.3 — Multi-Device I2C: DHT22 + OLED + Scan

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Multi-Device I2C Demo
 * DESKRIPSI: Mendemonstrasikan multiple perangkat pada satu bus I2C.
 *            Saat boot: scan bus → tampilkan daftar perangkat di OLED.
 *            Setelah itu: monitoring DHT22 normal di OLED.
 *
 * Tujuan: Mahasiswa memahami bahwa OLED dan sensor lain bisa
 *         berbagi bus I2C yang sama (SDA/SCL).
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(4, DHT22);

void scanAndDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("I2C Bus Scanner");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  int found = 0;
  int y = 13;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found++;
      display.setCursor(0, y);

      // Identifikasi perangkat umum
      if (addr == 0x3C || addr == 0x3D)
        display.printf("0x%02X: OLED SSD1306", addr);
      else if (addr == 0x76 || addr == 0x77)
        display.printf("0x%02X: BMP280/BME280", addr);
      else if (addr == 0x68)
        display.printf("0x%02X: MPU6050/DS3231", addr);
      else
        display.printf("0x%02X: Unknown device", addr);

      y += 10;
      Serial.printf("  [0x%02X] Ditemukan!\n", addr);
    }
  }

  display.setCursor(0, 55);
  display.printf("Total: %d perangkat", found);
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
    while(1) delay(1);
  }

  Serial.println("=== I2C Multi-Device Demo ===");
  scanAndDisplay();
  delay(4000);  // Tampilkan hasil scan selama 4 detik
}

void loop() {
  float suhu = dht.readTemperature();
  float hum  = dht.readHumidity();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("I2C Multi-Device OK");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  if (!isnan(suhu)) {
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.printf("%.1f C", suhu);
    display.setCursor(0, 35);
    display.printf("%.1f %%", hum);
  } else {
    display.setCursor(10, 25);
    display.println("Sensor Error");
  }

  display.setTextSize(1);
  display.setCursor(0, 56);
  display.printf("Up: %lus  I2C OK", millis()/1000);
  display.display();

  delay(2000);
}
```

---

## 4. Referensi

1. **Espressif.** *I2C API.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2c.html)
2. **Arduino.** *Wire Library.* [https://www.arduino.cc/reference/en/language/functions/communication/wire/](https://www.arduino.cc/reference/en/language/functions/communication/wire/)
3. **Adafruit.** *SSD1306 OLED.* [https://learn.adafruit.com/monochrome-oled-breakouts](https://learn.adafruit.com/monochrome-oled-breakouts)
4. **Random Nerd Tutorials.** *ESP32 I2C Tutorial.* [https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/](https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/)
