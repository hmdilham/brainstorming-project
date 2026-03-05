# Modul Pertemuan 3 — IOT Dasar
# GPIO Output — Digital & PWM (Pulse Width Modulation)

---

## 1. Maksud dan Tujuan Materi

### Maksud
GPIO (General Purpose Input/Output) adalah antarmuka utama mikrokontroler dengan dunia luar. Pertemuan ini mencakup dua jenis output: **Digital** (ON/OFF, 3.3V/0V) dan **PWM** (sinyal analog virtual dengan duty cycle variabel). Mahasiswa akan mengontrol LED eksternal dan LED RGB menggunakan kedua teknik ini.

### Tujuan Pembelajaran
1. **Menjelaskan** konsep GPIO dan konfigurasi pin (INPUT, OUTPUT, INPUT_PULLUP).
2. **Mengimplementasikan** output digital dengan `digitalWrite()` untuk mengendalikan LED.
3. **Memahami** Hukum Ohm dan menghitung resistor pembatas arus untuk LED.
4. **Menjelaskan** konsep PWM: duty cycle, frekuensi, resolusi.
5. **Menggunakan** LEDC API ESP32 (`ledcAttach`, `ledcWrite`) untuk LED dimming.
6. **Mengontrol** LED RGB dengan kombinasi PWM untuk menghasilkan warna.
7. **Menampilkan** status LED dan nilai PWM pada OLED display.

---

## 2. Teori Materi

### 2.1 GPIO ESP32

**GPIO** = pin serbaguna yang bisa dikonfigurasi sebagai input (membaca sinyal) atau output (mengeluarkan sinyal).

**Konfigurasi Pin:**
```cpp
pinMode(pin, MODE);
// MODE:
//   OUTPUT       → pin mengeluarkan tegangan (3.3V HIGH / 0V LOW)
//   INPUT        → pin membaca tegangan (floating, butuh resistor pull-up/down)
//   INPUT_PULLUP → pin membaca + resistor pull-up internal aktif
```

**Pin ESP32 yang Aman untuk Output:**
GPIO: 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33

> ⚠️ **Pin yang TIDAK boleh digunakan sembarangan:**
> - GPIO 6–11: Terhubung ke flash SPI internal (JANGAN GUNAKAN)
> - GPIO 34–39: Hanya INPUT (tidak bisa output)
> - GPIO 0, 2, 15: Pin strapping (memengaruhi boot mode)

### 2.2 Hukum Ohm & Perhitungan Resistor LED

Sebuah LED memiliki tegangan maju (forward voltage) dan arus kerja:
- LED merah: Vf ≈ 1.8V, If ≈ 20mA
- LED hijau: Vf ≈ 2.2V, If ≈ 20mA
- LED biru: Vf ≈ 3.0V, If ≈ 20mA

**Rumus Resistor Pembatas:**
```
R = (Vsumber - Vf) / If

Contoh LED merah dengan ESP32 (3.3V):
R = (3.3 - 1.8) / 0.020 = 75 Ω → gunakan 100Ω atau 220Ω (standar)
```

> **Praktis:** Gunakan resistor **220Ω** untuk semua jenis LED dengan ESP32 3.3V — cukup aman dan LED tetap terang.

### 2.3 PWM (Pulse Width Modulation)

PWM menghasilkan sinyal digital yang berkedip sangat cepat (ribuan kali per detik). Dengan mengatur rasio waktu ON vs OFF (duty cycle), kita dapat mengontrol "kecerahan rata-rata" yang diterima LED.

```
Duty Cycle 0%:    ___________________  → LED Mati
Duty Cycle 25%:   ████________________  → LED Redup
Duty Cycle 50%:   ████████____________  → LED Sedang
Duty Cycle 75%:   ████████████________  → LED Terang
Duty Cycle 100%:  ████████████████████  → LED Maksimum
```

**Parameter PWM:**
- **Frekuensi:** Berapa kali per detik sinyal berulang. 5000 Hz = 5000 siklus/detik.
- **Resolusi:** Berapa level detail duty cycle. 8-bit = 256 level (0–255). 12-bit = 4096 level.
- **Duty Cycle:** Persentase waktu sinyal HIGH. Pada 8-bit: 0=0%, 127≈50%, 255=100%.

### 2.4 LEDC API ESP32

ESP32 memiliki **16 channel LEDC** (LED Control) yang dirancang khusus untuk PWM.

**API Arduino-ESP32 v3.x (terbaru):**
```cpp
// Attach pin ke LEDC channel dengan frekuensi dan resolusi
ledcAttach(pin, frequency, resolution);

// Tulis duty cycle (0 hingga 2^resolusi - 1)
ledcWrite(pin, dutyCycle);

// Contoh: PWM 5000Hz, 8-bit resolusi pada GPIO 2
ledcAttach(2, 5000, 8);   // Setup
ledcWrite(2, 128);        // 50% duty cycle (128/255)
```

> **Catatan:** Pada versi lama Arduino-ESP32 (<3.0), API berbeda: `ledcSetup(channel, freq, res)` + `ledcAttachPin(pin, channel)`. Versi terbaru menyederhanakan ini.

### 2.5 LED RGB (Common Cathode)

LED RGB memiliki 3 LED (Merah, Hijau, Biru) dalam satu paket:
- **Common Cathode (CC):** Kaki paling panjang = GND. Setiap warna dikontrol dengan PWM ke kaki R/G/B.
- **Common Anode (CA):** Kaki paling panjang = 3.3V. Logika terbalik.

```
        Skema LED RGB Common Cathode
        
GPIO 25 ──[220Ω]──┤ R (merah)
GPIO 26 ──[220Ω]──┤ G (hijau)     LED RGB
GPIO 27 ──[220Ω]──┤ B (biru)      (Common Cathode)
          GND ─────┤ GND (kaki terpanjang)
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — LED Eksternal ON/OFF

**Kebutuhan:** ESP32, LED merah, Resistor 220Ω, Breadboard

**Skema Rangkaian:**
```
  ESP32 GPIO 2 ──── [220Ω] ──── LED (+anode) ──── LED (-katode) ──── GND
```

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
 * PROGRAM  : LED Eksternal ON/OFF + Blink Pattern
 * PLATFORM : PlatformIO + ESP32
 * DESKRIPSI: Mengendalikan LED eksternal pada GPIO 2.
 *            LED berkedip dengan pattern: nyala 1s, mati 500ms.
 */

#include <Arduino.h>

const uint8_t PIN_LED = 2;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  Serial.println("LED Eksternal — GPIO 2");
}

void loop() {
  digitalWrite(PIN_LED, HIGH);
  Serial.println("[LED] ON");
  delay(1000);

  digitalWrite(PIN_LED, LOW);
  Serial.println("[LED] OFF");
  delay(500);
}
```

---

### Praktikum 3.2 — LED Dimming (Fade) dengan PWM

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : LED Fade — PWM Dimming
 * DESKRIPSI: Kecerahan LED naik perlahan dari 0% ke 100%
 *            lalu turun kembali, menghasilkan efek "napas" (breathing).
 *            Menggunakan LEDC API ESP32.
 *
 * RANGKAIAN: GPIO 2 → [220Ω] → LED → GND
 */

#include <Arduino.h>

const uint8_t PIN_LED = 2;
const uint32_t PWM_FREQ = 5000;   // 5 kHz
const uint8_t  PWM_RES  = 8;     // 8-bit → 0-255

void setup() {
  Serial.begin(115200);
  ledcAttach(PIN_LED, PWM_FREQ, PWM_RES);
  Serial.println("LED Fade — PWM Breathing Effect");
}

void loop() {
  // Fade IN: 0 → 255
  for (int duty = 0; duty <= 255; duty += 5) {
    ledcWrite(PIN_LED, duty);
    Serial.printf("Brightness: %3d / 255 (%2d%%)\n", duty, duty * 100 / 255);
    delay(20);
  }

  // Fade OUT: 255 → 0
  for (int duty = 255; duty >= 0; duty -= 5) {
    ledcWrite(PIN_LED, duty);
    delay(20);
  }

  delay(500);  // Jeda sebelum siklus berikutnya
}
```

---

### Praktikum 3.3 — LED RGB: Pelangi dengan PWM

**Kebutuhan:** ESP32, LED RGB Common Cathode, 3× Resistor 220Ω

**Skema:**
```
  GPIO 25 ──[220Ω]── R (Red)
  GPIO 26 ──[220Ω]── G (Green)     LED RGB (CC)
  GPIO 27 ──[220Ω]── B (Blue)
  GND    ─────────── GND (kaki terpanjang)
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : LED RGB — Warna Pelangi dengan PWM
 * DESKRIPSI: Menghasilkan transisi warna pelangi halus pada LED RGB
 *            menggunakan 3 channel PWM (satu per warna).
 *
 * WARNA PELANGI: Merah → Kuning → Hijau → Cyan → Biru → Magenta → Merah
 */

#include <Arduino.h>

// Pin LED RGB
const uint8_t PIN_R = 25;
const uint8_t PIN_G = 26;
const uint8_t PIN_B = 27;

// PWM Config
const uint32_t PWM_FREQ = 5000;
const uint8_t  PWM_RES  = 8;

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(PIN_R, r);
  ledcWrite(PIN_G, g);
  ledcWrite(PIN_B, b);
  Serial.printf("RGB(%3d, %3d, %3d)\n", r, g, b);
}

void setup() {
  Serial.begin(115200);

  // Attach ketiga pin ke LEDC
  ledcAttach(PIN_R, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_G, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_B, PWM_FREQ, PWM_RES);

  Serial.println("LED RGB — Pelangi PWM");
}

void loop() {
  // Fase 1: Merah → Kuning (R=255, G naik)
  for (int i = 0; i <= 255; i += 5) { setRGB(255, i, 0); delay(15); }

  // Fase 2: Kuning → Hijau (R turun, G=255)
  for (int i = 255; i >= 0; i -= 5) { setRGB(i, 255, 0); delay(15); }

  // Fase 3: Hijau → Cyan (G=255, B naik)
  for (int i = 0; i <= 255; i += 5) { setRGB(0, 255, i); delay(15); }

  // Fase 4: Cyan → Biru (G turun, B=255)
  for (int i = 255; i >= 0; i -= 5) { setRGB(0, i, 255); delay(15); }

  // Fase 5: Biru → Magenta (R naik, B=255)
  for (int i = 0; i <= 255; i += 5) { setRGB(i, 0, 255); delay(15); }

  // Fase 6: Magenta → Merah (B turun, R=255)
  for (int i = 255; i >= 0; i -= 5) { setRGB(255, 0, i); delay(15); }
}
```

---

### Praktikum 3.4 — Versi OLED: Tampilkan Nilai PWM di Display

**Kebutuhan tambahan:** OLED SSD1306 128×64 (I2C)

**Skema Tambahan OLED:**
```
  OLED VCC → 3.3V
  OLED GND → GND
  OLED SDA → GPIO 21
  OLED SCL → GPIO 22
```

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
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : LED Fade + OLED Display
 * DESKRIPSI: LED breathing effect dengan nilai duty cycle dan
 *            persentase kecerahan ditampilkan real-time di OLED.
 *
 * RANGKAIAN:
 *   GPIO 2  → [220Ω] → LED → GND
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 *   OLED VCC → 3.3V, OLED GND → GND
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// LED PWM
const uint8_t  PIN_LED  = 2;
const uint32_t PWM_FREQ = 5000;
const uint8_t  PWM_RES  = 8;

void tampilkanOLED(int duty, const char* arah) {
  int persen = duty * 100 / 255;

  display.clearDisplay();

  // Judul
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LED PWM Dimming");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

  // Nilai duty cycle
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.printf("%d%%", persen);

  // Arah fade
  display.setTextSize(1);
  display.setCursor(80, 20);
  display.printf("(%s)", arah);

  // Progress bar
  display.drawRect(0, 40, 128, 12, SSD1306_WHITE);         // Outline
  int barWidth = map(duty, 0, 255, 0, 126);
  display.fillRect(1, 41, barWidth, 10, SSD1306_WHITE);     // Fill

  // Nilai numerik
  display.setCursor(0, 56);
  display.printf("Duty: %d/255", duty);

  display.display();
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
    while (true) delay(1);
  }
  display.clearDisplay();
  display.display();

  // Inisialisasi PWM
  ledcAttach(PIN_LED, PWM_FREQ, PWM_RES);

  Serial.println("LED Fade + OLED Display");
}

void loop() {
  // Fade IN
  for (int duty = 0; duty <= 255; duty += 5) {
    ledcWrite(PIN_LED, duty);
    tampilkanOLED(duty, "UP");
    delay(30);
  }

  // Fade OUT
  for (int duty = 255; duty >= 0; duty -= 5) {
    ledcWrite(PIN_LED, duty);
    tampilkanOLED(duty, "DOWN");
    delay(30);
  }

  delay(300);
}
```

---

## 4. Referensi

1. **Espressif.** *ESP32 GPIO & RTC GPIO.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
2. **Espressif.** *LED Control (LEDC) API.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html)
3. **Random Nerd Tutorials.** *ESP32 PWM with Arduino IDE.* [https://randomnerdtutorials.com/esp32-pwm-arduino-ide/](https://randomnerdtutorials.com/esp32-pwm-arduino-ide/)
4. **Adafruit.** *SSD1306 OLED Library.* [https://github.com/adafruit/Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
