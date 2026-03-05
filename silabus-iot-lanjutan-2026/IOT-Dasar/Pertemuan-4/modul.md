# Modul Pertemuan 4 — IOT Dasar
# GPIO Input — Digital & Analog (ADC)

---

## 1. Maksud dan Tujuan Materi

### Maksud
Setelah menguasai output, pertemuan ini membahas **input** — bagaimana ESP32 membaca sinyal dari dunia luar. Dua jenis input dipelajari: **Digital** (push button: HIGH/LOW) dan **Analog** (potensiometer: 0–4095 melalui ADC 12-bit). Konsep penting seperti debouncing dan limitasi ADC ESP32 juga dibahas.

### Tujuan Pembelajaran
1. **Membaca** input digital dari push button menggunakan `digitalRead()`.
2. **Mengimplementasikan** teknik debouncing software menggunakan `millis()`.
3. **Menjelaskan** perbedaan ADC ESP32 (12-bit) vs Arduino Uno (10-bit).
4. **Membaca** nilai analog dari potensiometer dengan `analogRead()`.
5. **Memahami** limitasi ADC ESP32: non-linearitas dan pin tertentu.
6. **Menampilkan** data input pada OLED display secara real-time.

---

## 2. Teori Materi

### 2.1 Input Digital — Push Button

Push button adalah saklar sesaat (momentary switch). Saat ditekan → menghubungkan dua terminal. Saat dilepas → terputus.

**Konfigurasi Pull-Up (Paling Umum):**

```
                 ESP32
  3.3V ──[10kΩ]──┤ GPIO 15 ──── [Push Button] ──── GND

  Button TIDAK ditekan: GPIO = HIGH (3.3V, terhubung ke 3.3V via pull-up)
  Button DITEKAN:       GPIO = LOW  (0V, terhubung langsung ke GND)
```

**Menggunakan Pull-Up Internal ESP32 (lebih praktis):**
```cpp
pinMode(15, INPUT_PULLUP);  // Aktifkan resistor pull-up internal (~45kΩ)
// Tidak perlu resistor eksternal!
// Button TIDAK ditekan → digitalRead() = HIGH (1)
// Button DITEKAN       → digitalRead() = LOW (0)
```

### 2.2 Masalah Bouncing & Solusi Debounce

Saat tombol ditekan, kontak mekanis **memantul** (bounce) beberapa kali dalam waktu 1–50 ms. Mikrokontroler yang sangat cepat (240 MHz) membaca pantulan ini sebagai beberapa tekanan.

```
Sinyal sebenarnya:    ────┐               ┌────
                          └───────────────┘

Sinyal bounce:        ────┐ ┌┐ ┌┐         ┌┐ ┌────
                          └─┘└─┘└─────────┘└─┘

                      ← bouncing →     ← bouncing →
                        (~5-50ms)         (~5-50ms)
```

**Solusi: Software Debounce dengan `millis()`:**
```cpp
const unsigned long DEBOUNCE_DELAY = 50; // 50ms
unsigned long lastDebounceTime = 0;
bool lastStableState = HIGH;
bool lastRawState = HIGH;

bool bacaTombolDebounced(uint8_t pin) {
  bool raw = digitalRead(pin);
  if (raw != lastRawState) {
    lastDebounceTime = millis();
  }
  lastRawState = raw;

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (raw != lastStableState) {
      lastStableState = raw;
      if (lastStableState == LOW) return true;  // Tombol baru ditekan
    }
  }
  return false;
}
```

### 2.3 ADC (Analog to Digital Converter), 12-bit

ADC mengubah tegangan analog (0V – 3.3V) menjadi nilai digital (0 – 4095).

| Parameter | Arduino Uno | ESP32 |
|---|---|---|
| Resolusi | 10-bit (0–1023) | **12-bit (0–4095)** |
| Tegangan Ref | 5V | **3.3V** |
| Channel | 6 (A0–A5) | **18 channel** (ADC1: GPIO 32–39, ADC2: lihat catatan) |
| Sampling Rate | ~10 kHz | **Hingga 2 MHz** |

**Pin ADC yang Aman:**
- **ADC1** (selalu tersedia): GPIO 32, 33, 34, 35, 36, 39
- **ADC2** (⚠️ tidak bisa digunakan saat WiFi aktif!): GPIO 0, 2, 4, 12–15, 25–27

> ⚠️ **PENTING:** Saat WiFi diaktifkan, ADC2 **tidak dapat digunakan** sama sekali. Jika membutuhkan ADC + WiFi, gunakan **hanya pin ADC1** (GPIO 32–39).

**Limitasi ADC ESP32:**
- Non-linear di ujung bawah (~0–100mV) dan atas (~3.1–3.3V)
- Untuk pengukuran presisi, gunakan kalibrasi atau ADC eksternal (ADS1115)

### 2.4 Potensiometer

Potensiometer adalah resistor variabel dengan 3 kaki:
- Kaki 1 → 3.3V
- Kaki 2 (tengah/wiper) → GPIO ADC
- Kaki 3 → GND

Memutar knob mengubah resistansi, menghasilkan tegangan 0V–3.3V pada pin wiper.

```
    3.3V ──── [Kaki 1]
                │
              ┌─┤ Potensiometer 10kΩ
              │ │
    GPIO 34 ──┤ [Kaki 2 / Wiper]
              │ │
              └─┤
                │
    GND  ──── [Kaki 3]
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Push Button Toggle LED (dengan Debounce)

**Kebutuhan:** ESP32, Push button, LED, Resistor 220Ω

**Skema:**
```
  GPIO 15 ── [Push Button] ── GND    (menggunakan INPUT_PULLUP internal)
  GPIO  2 ── [220Ω] ── LED ── GND
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
 * PROGRAM  : Push Button Toggle LED — Software Debounce
 * DESKRIPSI: Setiap kali tombol ditekan (dengan debounce), LED toggle
 *            ON/OFF. Counter jumlah tekanan ditampilkan di Serial Monitor.
 *
 * RANGKAIAN:
 *   GPIO 15 → Push Button → GND (INPUT_PULLUP)
 *   GPIO 2  → [220Ω] → LED → GND
 */

#include <Arduino.h>

const uint8_t PIN_BTN = 15;
const uint8_t PIN_LED = 2;

bool ledState = false;
int  pressCount = 0;

// Debounce variables
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_MS = 50;
bool lastRaw = HIGH;
bool lastStable = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);  // Pull-up internal
  pinMode(PIN_LED, OUTPUT);

  Serial.println("Push Button Toggle + Debounce");
  Serial.println("Tekan tombol untuk toggle LED");
}

void loop() {
  bool raw = digitalRead(PIN_BTN);

  // Reset timer jika sinyal berubah
  if (raw != lastRaw) {
    lastDebounceTime = millis();
  }
  lastRaw = raw;

  // Jika stabil selama DEBOUNCE_MS
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (raw != lastStable) {
      lastStable = raw;

      // Deteksi falling edge (HIGH → LOW = tombol ditekan)
      if (lastStable == LOW) {
        ledState = !ledState;
        pressCount++;
        digitalWrite(PIN_LED, ledState);

        Serial.printf("[#%d] Tombol ditekan! LED = %s\n",
                      pressCount, ledState ? "ON ████" : "OFF ░░░░");
      }
    }
  }
}
```

---

### Praktikum 3.2 — Potensiometer ADC → LED Dimming

**Kebutuhan:** ESP32, Potensiometer 10kΩ, LED, Resistor 220Ω

**Skema:**
```
  Potensio kaki1 → 3.3V | kaki2 (wiper) → GPIO 34 | kaki3 → GND
  GPIO 2 → [220Ω] → LED → GND
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Potensiometer ADC → LED Dimming
 * DESKRIPSI: Membaca nilai analog potensiometer (12-bit ADC: 0-4095)
 *            dan menggunakannya untuk mengontrol kecerahan LED via PWM.
 *
 * RANGKAIAN:
 *   GPIO 34 → Potensiometer wiper (ADC1 — aman dengan WiFi)
 *   GPIO 2  → [220Ω] → LED → GND
 */

#include <Arduino.h>

const uint8_t PIN_POT = 34;   // ADC1 channel
const uint8_t PIN_LED = 2;

void setup() {
  Serial.begin(115200);

  // Setup PWM untuk LED
  ledcAttach(PIN_LED, 5000, 8);  // 5kHz, 8-bit

  // ADC resolution (default sudah 12-bit, tapi eksplisit lebih baik)
  analogReadResolution(12);

  Serial.println("Potentiometer ADC → LED Dimming");
  Serial.println("Putar potensiometer untuk mengatur kecerahan LED");
}

void loop() {
  // Baca ADC (0-4095)
  int adcValue = analogRead(PIN_POT);

  // Konversi ke tegangan
  float voltage = adcValue * 3.3 / 4095.0;

  // Map ke range PWM 8-bit (0-255)
  int pwmValue = map(adcValue, 0, 4095, 0, 255);
  int persen = adcValue * 100 / 4095;

  // Atur kecerahan LED
  ledcWrite(PIN_LED, pwmValue);

  // Tampilkan di Serial Monitor
  Serial.printf("ADC: %4d | Tegangan: %.2fV | PWM: %3d | Brightness: %d%%\n",
                adcValue, voltage, pwmValue, persen);

  delay(100);
}
```

---

### Praktikum 3.3 — Versi OLED: Tampilkan ADC + LED Status

**Kebutuhan tambahan:** OLED SSD1306 128×64 (I2C)

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
 * PROGRAM  : ADC + LED Dimming + OLED Display
 * DESKRIPSI: Potensiometer mengontrol LED brightness.
 *            OLED menampilkan: nilai ADC, tegangan, persentase,
 *            progress bar visual, dan status push button.
 *
 * RANGKAIAN:
 *   GPIO 34 → Potensiometer wiper
 *   GPIO 15 → Push Button → GND (INPUT_PULLUP)
 *   GPIO 2  → [220Ω] → LED → GND
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const uint8_t PIN_POT = 34;
const uint8_t PIN_BTN = 15;
const uint8_t PIN_LED = 2;

bool ledEnabled = true;
int pressCount = 0;

// Debounce
unsigned long lastDb = 0;
bool lastRaw = HIGH, lastStable = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  ledcAttach(PIN_LED, 5000, 8);
  analogReadResolution(12);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
    while (true) delay(1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  Serial.println("ADC + Button + LED + OLED");
}

void loop() {
  // === Debounce Button ===
  bool raw = digitalRead(PIN_BTN);
  if (raw != lastRaw) lastDb = millis();
  lastRaw = raw;
  if ((millis() - lastDb) > 50 && raw != lastStable) {
    lastStable = raw;
    if (lastStable == LOW) {
      ledEnabled = !ledEnabled;
      pressCount++;
    }
  }

  // === Baca ADC ===
  int adc = analogRead(PIN_POT);
  float volt = adc * 3.3 / 4095.0;
  int pwm = map(adc, 0, 4095, 0, 255);
  int persen = adc * 100 / 4095;

  // === LED Control ===
  ledcWrite(PIN_LED, ledEnabled ? pwm : 0);

  // === OLED Display ===
  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("GPIO Input Monitor");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // ADC Value
  display.setCursor(0, 13);
  display.printf("ADC : %4d / 4095", adc);

  // Voltage
  display.setCursor(0, 23);
  display.printf("Volt: %.2f V", volt);

  // Brightness bar
  display.setCursor(0, 33);
  display.printf("LED : %d%%", ledEnabled ? persen : 0);
  display.drawRect(60, 33, 68, 8, SSD1306_WHITE);
  if (ledEnabled) {
    display.fillRect(61, 34, map(pwm, 0, 255, 0, 66), 6, SSD1306_WHITE);
  }

  // Button status
  display.setCursor(0, 45);
  display.printf("Btn : %s (#%d)", ledEnabled ? "ON" : "OFF", pressCount);

  // Status bar
  display.drawLine(0, 55, 128, 55, SSD1306_WHITE);
  display.setCursor(0, 57);
  display.printf("PWM=%d  %s", pwm, ledEnabled ? "ACTIVE" : "PAUSED");

  display.display();

  delay(50);  // ~20 FPS update rate
}
```

---

## 4. Referensi

1. **Espressif.** *ADC API Reference.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc_oneshot.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc_oneshot.html)
2. **Random Nerd Tutorials.** *ESP32 ADC.* [https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/](https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/)
3. **Arduino.** *digitalRead() Reference.* [https://www.arduino.cc/reference/en/language/functions/digital-io/digitalread/](https://www.arduino.cc/reference/en/language/functions/digital-io/digitalread/)
4. **Arduino.** *Debounce Tutorial.* [https://docs.arduino.cc/built-in-examples/digital/Debounce/](https://docs.arduino.cc/built-in-examples/digital/Debounce/)
