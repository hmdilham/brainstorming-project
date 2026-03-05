# Modul Pertemuan 8 — IOT Dasar
# Sensor — Mengumpulkan Data Lingkungan

---

## 1. Maksud dan Tujuan Materi

### Maksud
Sensor adalah "mata dan telinga" sistem IoT — mengubah besaran fisik menjadi sinyal listrik yang dapat dibaca mikrokontroler. Pertemuan ini membahas empat jenis sensor populer: **PIR** (gerakan), **LDR** (cahaya), **HC-SR04** (jarak ultrasonik), dan **BMP280** (suhu + tekanan barometrik).

### Tujuan Pembelajaran
1. **Membaca** sensor digital PIR (HIGH/LOW) untuk deteksi gerakan.
2. **Membaca** sensor analog LDR dengan pembagi tegangan dan kalibrasi threshold.
3. **Mengukur** jarak dengan HC-SR04 (echo timing, rumus kecepatan suara).
4. **Membaca** suhu dan tekanan dari BMP280 via I2C.
5. **Menampilkan** semua data sensor pada OLED secara bersamaan.

---

## 2. Teori Materi

### 2.1 Sensor PIR HC-SR501 (Gerakan)

Sensor PIR (Passive Infrared) mendeteksi perubahan radiasi inframerah dari benda bergerak yang bersuhu (manusia, hewan).

| Parameter | Nilai |
|---|---|
| Tegangan | 5V – 20V (output tetap 3.3V — aman untuk ESP32!) |
| Jangkauan | Hingga 7 meter |
| Sudut deteksi | ~120° |
| Output | Digital: HIGH saat gerakan terdeteksi |
| Delay | Adjustable via potensiometer on-board (1–300 detik) |

**Koneksi:** VCC → 5V (dari VIN), OUT → GPIO, GND → GND

### 2.2 LDR — Light Dependent Resistor (Cahaya)

LDR mengubah intensitas cahaya menjadi resistansi:
- **Terang:** Resistansi rendah (~1 kΩ)
- **Gelap:** Resistansi tinggi (~100+ kΩ)

**Pembagi Tegangan dengan LDR:**
```
  3.3V ──── [LDR] ──┬── GPIO 34 (ADC)
                     │
                   [10kΩ]
                     │
  GND  ──────────────┘
  
  Terang → LDR rendah → tegangan ADC tinggi → ADC value tinggi
  Gelap  → LDR tinggi → tegangan ADC rendah → ADC value rendah
```

### 2.3 HC-SR04 — Sensor Ultrasonik (Jarak)

Memancarkan gelombang ultrasonik 40 kHz dan mengukur waktu pantulan.

```
  Rumus: Jarak (cm) = durasi_echo (µs) × 0.0343 / 2

  0.0343 cm/µs = kecepatan suara di udara (343 m/s = 34300 cm/s)
  Dibagi 2 karena suara pergi-pulang
```

| Parameter | Nilai |
|---|---|
| Tegangan | **5V** (output ECHO bisa 5V — butuh voltage divider!) |
| Jangkauan | 2 cm – 400 cm |
| Akurasi | ± 3 mm |
| Sudut | ~15° |

> ⚠️ HC-SR04 beroperasi di **5V**. Pin ECHO bisa output 5V! Gunakan voltage divider (2 resistor) untuk menurunkan ke 3.3V sebelum masuk GPIO ESP32.

**Voltage Divider untuk ECHO:**
```
  HC-SR04 ECHO ──[1kΩ]──┬── GPIO 33 (input)
                         │
                       [2kΩ]
                         │
                        GND
  
  Vout = 5V × 2kΩ / (1kΩ + 2kΩ) = 3.33V ✅ (aman untuk ESP32)
```

### 2.4 BMP280 — Sensor Barometrik (Suhu + Tekanan)

| Parameter | Nilai |
|---|---|
| Suhu | -40°C ~ +85°C (±1°C) |
| Tekanan | 300 – 1100 hPa (±1 hPa) |
| Interface | I2C (0x76 atau 0x77) |
| Tegangan | 1.8V – 3.6V |
| Bisa hitung | Estimasi ketinggian dari tekanan |

---

## 3. Panduan Praktikum

### Praktikum 3.1 — PIR: Deteksi Gerakan + LED Alarm

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
 * PROGRAM  : PIR Motion Detector + LED Alarm
 * DESKRIPSI: Membaca sensor PIR. Jika gerakan terdeteksi,
 *            nyalakan LED selama 5 detik + pesan ke Serial.
 *
 * RANGKAIAN:
 *   PIR OUT → GPIO 13
 *   PIR VCC → 5V (VIN)
 *   PIR GND → GND
 *   GPIO 2  → [220Ω] → LED → GND
 */

#include <Arduino.h>

const uint8_t PIN_PIR = 13;
const uint8_t PIN_LED = 2;

int motionCount = 0;
unsigned long ledOffTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LED, OUTPUT);

  Serial.println("PIR Motion Detector");
  Serial.println("Menunggu gerakan...");
  delay(3000); // PIR warmup (~30 detik ideal, 3 detik minimum)
}

void loop() {
  if (digitalRead(PIN_PIR) == HIGH) {
    if (ledOffTime == 0 || millis() > ledOffTime) {
      motionCount++;
      digitalWrite(PIN_LED, HIGH);
      ledOffTime = millis() + 5000; // LED nyala 5 detik
      Serial.printf("[#%d] GERAKAN TERDETEKSI! LED ON 5 detik\n", motionCount);
    }
  }

  if (ledOffTime > 0 && millis() > ledOffTime) {
    digitalWrite(PIN_LED, LOW);
    ledOffTime = 0;
    Serial.println("       LED OFF — menunggu...");
  }

  delay(100);
}
```

---

### Praktikum 3.2 — HC-SR04: Jarak + Buzzer Peringatan

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : HC-SR04 Jarak + Buzzer Alarm Proximity
 * DESKRIPSI: Mengukur jarak objek dengan HC-SR04 ultrasonik.
 *            Buzzer berbunyi jika objek < 20cm. Pitch buzzer
 *            meningkat seiring objek mendekat.
 *
 * RANGKAIAN:
 *   HC-SR04 TRIG → GPIO 32
 *   HC-SR04 ECHO → [Voltage Divider 1kΩ+2kΩ] → GPIO 33
 *   HC-SR04 VCC  → 5V
 *   Buzzer       → GPIO 25
 */

#include <Arduino.h>

const uint8_t PIN_TRIG   = 32;
const uint8_t PIN_ECHO   = 33;
const uint8_t PIN_BUZZER = 25;

float ukurJarak() {
  // Kirim pulse 10µs
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Ukur durasi echo
  long durasi = pulseIn(PIN_ECHO, HIGH, 30000); // timeout 30ms (~5m)

  if (durasi == 0) return -1; // Timeout — tidak ada objek

  return durasi * 0.0343 / 2.0;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  ledcAttach(PIN_BUZZER, 2000, 8);

  Serial.println("HC-SR04 Proximity Alarm");
}

void loop() {
  float jarak = ukurJarak();

  if (jarak < 0) {
    Serial.println("Tidak ada objek terdeteksi");
    ledcWrite(PIN_BUZZER, 0); // Buzzer OFF
  } else {
    // Bar visual
    int bar = constrain(map((int)jarak, 0, 100, 20, 0), 0, 20);
    Serial.printf("Jarak: %6.1f cm [", jarak);
    for (int i = 0; i < 20; i++) Serial.print(i < bar ? "█" : "░");
    Serial.println("]");

    // Buzzer alarm jika < 20cm
    if (jarak < 20) {
      int freq = map(constrain((int)jarak, 2, 20), 20, 2, 500, 3000);
      ledcAttach(PIN_BUZZER, freq, 8);
      ledcWrite(PIN_BUZZER, 128);
    } else {
      ledcWrite(PIN_BUZZER, 0);
    }
  }

  delay(100);
}
```

---

### Praktikum 3.3 — Versi OLED: Multi-Sensor Dashboard

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit BMP280 Library@^2.6.8
    adafruit/Adafruit Unified Sensor@^1.1.14
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Multi-Sensor Dashboard + OLED
 * DESKRIPSI: Menggabungkan LDR + HC-SR04 + BMP280 dan menampilkan
 *            semua data di OLED display secara real-time.
 *
 * RANGKAIAN:
 *   GPIO 34 → LDR + 10kΩ voltage divider
 *   GPIO 32 → HC-SR04 TRIG
 *   GPIO 33 → HC-SR04 ECHO (via voltage divider)
 *   GPIO 21 → OLED SDA / BMP280 SDA
 *   GPIO 22 → OLED SCL / BMP280 SCL
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Adafruit_BMP280 bmp;

const uint8_t PIN_LDR  = 34;
const uint8_t PIN_TRIG = 32;
const uint8_t PIN_ECHO = 33;

bool bmpOK = false;

float ukurJarak() {
  digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long d = pulseIn(PIN_ECHO, HIGH, 30000);
  return (d == 0) ? -1 : d * 0.0343 / 2.0;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  analogReadResolution(12);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }

  bmpOK = bmp.begin(0x76);
  if (!bmpOK) {
    bmpOK = bmp.begin(0x77); // Coba alamat alternatif
  }
  Serial.printf("BMP280: %s\n", bmpOK ? "OK" : "Tidak ditemukan");

  display.clearDisplay(); display.display();
}

void loop() {
  // === Baca Sensor ===
  int ldrRaw = analogRead(PIN_LDR);
  int ldrPersen = map(ldrRaw, 0, 4095, 0, 100);
  float jarak = ukurJarak();
  float suhu = bmpOK ? bmp.readTemperature() : -99;
  float tekanan = bmpOK ? bmp.readPressure() / 100.0 : -1; // hPa
  float altitude = bmpOK ? bmp.readAltitude(1013.25) : -1;

  // === OLED Display ===
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(0, 0);
  display.println("Multi-Sensor Monitor");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Cahaya (LDR)
  display.setCursor(0, 12);
  display.printf("Cahaya: %d%%", ldrPersen);
  display.drawRect(75, 12, 53, 7, SSD1306_WHITE);
  display.fillRect(76, 13, map(ldrPersen, 0, 100, 0, 51), 5, SSD1306_WHITE);

  // Jarak (HC-SR04)
  display.setCursor(0, 22);
  if (jarak > 0)
    display.printf("Jarak : %.1f cm", jarak);
  else
    display.printf("Jarak : ---");

  // Suhu + Tekanan (BMP280)
  display.setCursor(0, 32);
  if (bmpOK) {
    display.printf("Suhu  : %.1f C", suhu);
    display.setCursor(0, 42);
    display.printf("Tekan : %.0f hPa", tekanan);
    display.setCursor(0, 52);
    display.printf("Alti  : %.0f m", altitude);
  } else {
    display.printf("BMP280: N/A");
  }

  display.display();

  // Serial output
  Serial.printf("LDR=%d%% | Jarak=%.1fcm | Suhu=%.1fC | P=%.0fhPa\n",
                ldrPersen, jarak, suhu, tekanan);

  delay(200);
}
```

---

## 4. Referensi

1. **Random Nerd Tutorials.** *HC-SR04 Ultrasonic Sensor.* [https://randomnerdtutorials.com/esp32-hc-sr04-ultrasonic-arduino/](https://randomnerdtutorials.com/esp32-hc-sr04-ultrasonic-arduino/)
2. **Adafruit.** *BMP280 Library.* [https://github.com/adafruit/Adafruit_BMP280_Library](https://github.com/adafruit/Adafruit_BMP280_Library)
3. **Last Minute Engineers.** *PIR Motion Sensor.* [https://lastminuteengineers.com/pir-sensor-arduino-tutorial/](https://lastminuteengineers.com/pir-sensor-arduino-tutorial/)
