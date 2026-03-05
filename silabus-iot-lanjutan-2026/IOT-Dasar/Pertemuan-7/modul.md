# Modul Pertemuan 7 — IOT Dasar
# Protokol Komunikasi — SPI (ESP32 ↔ Modul Penyimpanan)

---

## 1. Maksud dan Tujuan Materi

### Maksud
SPI (Serial Peripheral Interface) adalah protokol komunikasi serial **sinkron full-duplex** yang sangat cepat. SPI digunakan untuk perangkat yang membutuhkan kecepatan transfer tinggi: SD Card, layar TFT, flash memory, dan ADC eksternal. Pertemuan ini fokus pada aplikasi praktis utama SPI di IoT: **data logging sensor ke SD Card**.

### Tujuan Pembelajaran
1. **Menjelaskan** arsitektur SPI: MOSI, MISO, SCLK, CS, full-duplex.
2. **Membedakan** SPI vs I2C: kapan menggunakan yang mana.
3. **Menginisialisasi** modul SD Card via SPI pada ESP32.
4. **Mengimplementasikan** data logger: menulis dan membaca data sensor ke/dari file SD Card.
5. **Menampilkan** status logging pada OLED display.

---

## 2. Teori Materi

### 2.1 Arsitektur SPI

```
          MASTER (ESP32)              SLAVE (SD Card)
        ┌──────────────┐            ┌──────────────┐
        │         MOSI ├───────────►│ MOSI (DI)    │  Data: Master → Slave
        │         MISO │◄───────────┤ MISO (DO)    │  Data: Slave → Master
        │         SCLK ├───────────►│ SCLK (CLK)   │  Clock (dikendalikan Master)
        │           CS ├───────────►│ CS (SS)      │  Chip Select (aktif LOW)
        └──────────────┘            └──────────────┘
```

**Perbandingan SPI vs I2C:**

| Aspek | I2C | SPI |
|---|---|---|
| Kabel | 2 (SDA + SCL) | 4 (MOSI + MISO + SCLK + CS) |
| Kecepatan | 100–400 kbps | **Hingga 80 MHz di ESP32!** |
| Duplex | Half-duplex | **Full-duplex** ✅ |
| Max devices | 127 (via alamat) | Terbatas pin CS |
| Addressing | Software (7-bit) | Hardware (pin CS per device) |
| Ideal untuk | Sensor lambat, OLED | **SD Card, Display TFT, Flash** |

### 2.2 Pin SPI Default ESP32

| Sinyal | VSPI (default) | HSPI |
|---|---|---|
| MOSI | GPIO 23 | GPIO 13 |
| MISO | GPIO 19 | GPIO 12 |
| SCLK | GPIO 18 | GPIO 14 |
| CS | GPIO 5 | GPIO 15 |

### 2.3 Modul SD Card

Modul SD Card SPI adapter menghubungkan kartu MicroSD ke bus SPI:

```
  Modul SD Card        ESP32 (VSPI)
  ┌───────────┐
  │     CS    ├──── GPIO 5
  │    MOSI   ├──── GPIO 23
  │    MISO   ├──── GPIO 19
  │    SCLK   ├──── GPIO 18
  │    VCC    ├──── 3.3V (atau 5V jika modul ada regulator)
  │    GND    ├──── GND
  └───────────┘
```

> ⚠️ **Format SD Card:** Harus **FAT32**. Kartu >32GB (SDXC) perlu diformat ulang ke FAT32 karena default format-nya exFAT.

---

## 3. Panduan Praktikum

### Praktikum 3.1 — SD Card: Tulis dan Baca File

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
 * PROGRAM  : SD Card — Tulis & Baca File
 * DESKRIPSI: Inisialisasi SD Card via SPI, menulis teks ke file,
 *            membaca kembali, dan menampilkan info kartu SD.
 *
 * RANGKAIAN:
 *   SD CS   → GPIO 5
 *   SD MOSI → GPIO 23
 *   SD MISO → GPIO 19
 *   SD SCLK → GPIO 18
 *   SD VCC  → 3.3V
 *   SD GND  → GND
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

const uint8_t PIN_CS = 5;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("═══════════════════════════");
  Serial.println("  SD Card SPI Demo");
  Serial.println("═══════════════════════════");

  // Inisialisasi SD Card
  if (!SD.begin(PIN_CS)) {
    Serial.println("[ERR] SD Card gagal! Cek:");
    Serial.println("  - Kartu terpasang?");
    Serial.println("  - Format FAT32?");
    Serial.println("  - Kabel CS benar (GPIO 5)?");
    while (true) delay(1);
  }

  // Info kartu
  uint8_t cardType = SD.cardType();
  Serial.printf("Tipe  : %s\n",
    cardType == CARD_MMC ? "MMC" :
    cardType == CARD_SD  ? "SDSC" :
    cardType == CARD_SDHC ? "SDHC" : "Unknown");
  Serial.printf("Ukuran: %llu MB\n", SD.cardSize() / (1024 * 1024));
  Serial.printf("Ruang : %llu MB terpakai\n", SD.usedBytes() / (1024 * 1024));

  // === TULIS FILE ===
  File f = SD.open("/test.txt", FILE_WRITE);
  if (f) {
    f.println("Hello dari ESP32!");
    f.printf("Waktu boot: %lu ms\n", millis());
    f.println("SD Card SPI berhasil.");
    f.close();
    Serial.println("\n[OK] File /test.txt ditulis.");
  } else {
    Serial.println("[ERR] Gagal buka file untuk tulis!");
  }

  // === BACA FILE ===
  f = SD.open("/test.txt", FILE_READ);
  if (f) {
    Serial.println("\n--- Isi /test.txt ---");
    while (f.available()) {
      Serial.write(f.read());
    }
    Serial.println("--- Akhir file ---");
    f.close();
  }
}

void loop() {
  delay(10000);
}
```

---

### Praktikum 3.2 — Data Logger: DHT22 → SD Card + OLED

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
 * PROGRAM  : IoT Data Logger — DHT22 → SD Card + OLED
 * DESKRIPSI: Membaca sensor DHT22 setiap 5 detik, menyimpan data
 *            ke file CSV di SD Card, dan menampilkan status di OLED.
 *
 * FORMAT CSV: timestamp_ms, sample_num, suhu_c, kelembaban_persen
 *
 * RANGKAIAN:
 *   GPIO 4  → DHT22 DATA
 *   GPIO 5  → SD CS
 *   GPIO 18 → SD SCLK
 *   GPIO 19 → SD MISO
 *   GPIO 23 → SD MOSI
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(4, DHT22);

const uint8_t PIN_CS = 5;
const char* LOG_FILE = "/sensor_log.csv";
const unsigned long LOG_INTERVAL = 5000; // 5 detik

int sampleCount = 0;
bool sdOK = false;
unsigned long lastLog = 0;

void updateOLED(float suhu, float hum, bool logOK) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(0, 0);
  display.printf("Data Logger  #%d", sampleCount);
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Sensor data
  display.setTextSize(2);
  display.setCursor(0, 14);
  display.printf("%.1fC", suhu);
  display.setCursor(75, 14);
  display.printf("%.0f%%", hum);

  // Status SD Card
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.printf("SD: %s", sdOK ? "OK" : "GAGAL");

  // Status log
  display.setCursor(0, 45);
  display.printf("Log: %s", logOK ? "TERSIMPAN" : "GAGAL");

  // File info
  display.setCursor(0, 55);
  if (sdOK) {
    File f = SD.open(LOG_FILE, FILE_READ);
    if (f) {
      display.printf("File: %lu bytes", f.size());
      f.close();
    }
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 25);
  display.setTextSize(1);
  display.println("Init Data Logger...");
  display.display();

  // SD Card
  sdOK = SD.begin(PIN_CS);
  if (sdOK) {
    Serial.println("[SD] OK!");
    // Tulis header CSV jika file belum ada
    if (!SD.exists(LOG_FILE)) {
      File f = SD.open(LOG_FILE, FILE_WRITE);
      if (f) {
        f.println("timestamp_ms,sample,suhu_c,kelembaban_persen");
        f.close();
      }
    }
  } else {
    Serial.println("[SD] GAGAL!");
  }

  Serial.println("Data Logger siap.");
  delay(1000);
}

void loop() {
  if (millis() - lastLog < LOG_INTERVAL) return;
  lastLog = millis();

  float suhu = dht.readTemperature();
  float hum  = dht.readHumidity();
  bool logOK = false;

  if (!isnan(suhu) && !isnan(hum)) {
    sampleCount++;

    // Simpan ke SD Card
    if (sdOK) {
      File f = SD.open(LOG_FILE, FILE_APPEND);
      if (f) {
        f.printf("%lu,%d,%.1f,%.1f\n", millis(), sampleCount, suhu, hum);
        f.close();
        logOK = true;
      }
    }

    Serial.printf("[#%d] %.1f°C | %.1f%% | SD: %s\n",
                  sampleCount, suhu, hum, logOK ? "OK" : "FAIL");
  }

  updateOLED(suhu, hum, logOK);
}
```

---

## 4. Referensi

1. **Espressif.** *SPI Master API.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html)
2. **Arduino.** *SD Library.* [https://www.arduino.cc/reference/en/libraries/sd/](https://www.arduino.cc/reference/en/libraries/sd/)
3. **Random Nerd Tutorials.** *ESP32 SD Card.* [https://randomnerdtutorials.com/esp32-microsd-card-arduino/](https://randomnerdtutorials.com/esp32-microsd-card-arduino/)
