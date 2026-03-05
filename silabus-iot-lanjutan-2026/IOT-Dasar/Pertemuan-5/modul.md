# Modul Pertemuan 5 — IOT Dasar
# Protokol Komunikasi — UART (ESP32 ↔ Sensor/Modul)

---

## 1. Maksud dan Tujuan Materi

### Maksud
UART (Universal Asynchronous Receiver-Transmitter) adalah protokol komunikasi serial paling fundamental. Pada konteks IOT Dasar, UART digunakan untuk komunikasi antara **ESP32 dengan komputer** (Serial Monitor) dan **ESP32 dengan modul eksternal** (GPS, GSM, sensor UART). Komunikasi antar-ESP32 via UART dibahas di mata kuliah IOT Lanjutan.

> **Catatan Konteks:** Di IOT Dasar (P5–P7), protokol UART/I2C/SPI digunakan untuk ESP32 ↔ **sensor/modul**. Di IOT Lanjutan (P1–P2), protokol yang **sama** digunakan untuk ESP32 ↔ **ESP32 lain**.

### Tujuan Pembelajaran
1. **Menjelaskan** prinsip komunikasi UART: TX, RX, baud rate, start/stop bit, paritas.
2. **Mengidentifikasi** 3 port UART pada ESP32 dan fungsinya.
3. **Mengimplementasikan** komunikasi dua arah: ESP32 ↔ Serial Monitor.
4. **Membangun** parser perintah string untuk kontrol LED via Serial.
5. **Menggunakan** UART2 (`Serial2`) untuk komunikasi dengan modul eksternal.
6. **Menampilkan** data UART pada OLED display.

---

## 2. Teori Materi

### 2.1 Prinsip Kerja UART

UART adalah komunikasi serial **asinkron** — tidak ada sinyal clock bersama. Pengirim dan penerima harus menyepakati kecepatan (baud rate) sebelumnya.

**Struktur Frame UART:**

```
Idle ──┐   ┌── Start Bit (LOW)
       │   │
       ▼   ▼
  ─────┐ ┌─┬───┬───┬───┬───┬───┬───┬───┬───┬─┬─────
  HIGH │ │0│ D0│ D1│ D2│ D3│ D4│ D5│ D6│ D7│1│ HIGH
       └─┘ └───┴───┴───┴───┴───┴───┴───┴───┘ │
         S   ←─── 8 Data Bits ──────────────→ Stop
         t                                     Bit
         a
         r
         t

  Total: 1 start + 8 data + 1 stop = 10 bit per karakter
```

**Parameter UART:**

| Parameter | Nilai Umum | Keterangan |
|---|---|---|
| **Baud Rate** | 9600, 115200 | bit/detik. 115200 = 11.520 byte/detik |
| **Data Bits** | 8 | Jumlah bit data per frame |
| **Stop Bits** | 1 | Bit penanda akhir frame |
| **Parity** | None | Opsional error detection (Even/Odd) |
| **Format** | 8N1 | 8 data, No parity, 1 stop bit (standar) |

### 2.2 UART pada ESP32

ESP32 memiliki **3 port UART hardware:**

| Port | Default TX | Default RX | Fungsi |
|---|---|---|---|
| **UART0** | GPIO 1 | GPIO 3 | USB-Serial (Serial Monitor) — **jangan diubah** |
| **UART1** | GPIO 10 | GPIO 9 | Terhubung ke flash SPI — **perlu remap pin** |
| **UART2** | GPIO 17 | GPIO 16 | **Bebas digunakan** untuk modul eksternal ✅ |

```cpp
// UART0 — komunikasi dengan komputer
Serial.begin(115200);
Serial.println("Hello dari UART0");

// UART2 — komunikasi dengan modul eksternal (GPS, GSM, dll.)
Serial2.begin(9600, SERIAL_8N1, 16, 17);  // RX=GPIO16, TX=GPIO17
Serial2.println("AT");  // Kirim perintah AT ke modul
```

### 2.3 Fungsi Serial yang Penting

| Fungsi | Deskripsi |
|---|---|
| `Serial.begin(baud)` | Mulai komunikasi UART dengan baud rate tertentu |
| `Serial.print(data)` | Kirim data tanpa newline |
| `Serial.println(data)` | Kirim data dengan newline `\r\n` |
| `Serial.printf(format, ...)` | Kirim data terformat (seperti C printf) |
| `Serial.available()` | Cek berapa byte tersedia untuk dibaca |
| `Serial.read()` | Baca 1 byte. Return -1 jika kosong |
| `Serial.readString()` | Baca semua byte hingga timeout sebagai String |
| `Serial.readStringUntil('\n')` | Baca hingga karakter delimiter tertentu |
| `Serial.parseInt()` | Baca dan parse integer dari stream |
| `Serial.parseFloat()` | Baca dan parse float dari stream |
| `Serial.flush()` | Tunggu sampai semua data terkirim |
| `Serial.setTimeout(ms)` | Atur timeout untuk readString/parseInt (default 1000ms) |

### 2.4 Teknik Parsing Serial

**Metode 1: Karakter Tunggal**
```cpp
if (Serial.available()) {
  char c = Serial.read();
  if (c == '1') digitalWrite(LED, HIGH);
  if (c == '0') digitalWrite(LED, LOW);
}
```

**Metode 2: String dengan Delimiter**
```cpp
if (Serial.available()) {
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();  // Hapus whitespace/newline

  if (cmd == "LED:ON")  digitalWrite(LED, HIGH);
  if (cmd == "LED:OFF") digitalWrite(LED, LOW);
  if (cmd.startsWith("PWM:")) {
    int val = cmd.substring(4).toInt();
    ledcWrite(LED, val);
  }
}
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Kirim Data Sensor ke Serial Monitor

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
 * PROGRAM  : Sensor Data → Serial Monitor
 * DESKRIPSI: Membaca potensiometer (ADC) dan mengirim data terformat
 *            ke Serial Monitor setiap 1 detik. Menampilkan timestamp,
 *            nilai ADC, tegangan, dan interpretasi level.
 *
 * RANGKAIAN: GPIO 34 → Potensiometer wiper
 */

#include <Arduino.h>

const uint8_t PIN_POT = 34;
uint32_t sampleCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  analogReadResolution(12);

  Serial.println("╔══════════════════════════════════════════╗");
  Serial.println("║  ESP32 — Sensor Data Logger (Serial)     ║");
  Serial.println("╠══════════════════════════════════════════╣");
  Serial.println("║  #    | Waktu(s) | ADC  | Volt | Level   ║");
  Serial.println("╚══════════════════════════════════════════╝");
}

void loop() {
  sampleCount++;
  int adc = analogRead(PIN_POT);
  float volt = adc * 3.3 / 4095.0;

  // Interpretasi level
  const char* level;
  if (adc < 1000)      level = "RENDAH ";
  else if (adc < 3000) level = "SEDANG ";
  else                 level = "TINGGI ";

  // Kirim data terformat
  Serial.printf("  %4d | %7lu  | %4d | %.2f | %s",
                sampleCount, millis()/1000, adc, volt, level);

  // Bar visual 20 karakter
  int bar = map(adc, 0, 4095, 0, 20);
  Serial.print("[");
  for (int i = 0; i < 20; i++) Serial.print(i < bar ? "█" : "░");
  Serial.println("]");

  delay(1000);
}
```

---

### Praktikum 3.2 — Kontrol LED via Perintah Serial

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Parser Perintah Serial → Kontrol LED
 * DESKRIPSI: Menerima perintah dari Serial Monitor untuk mengontrol LED.
 *            Perintah yang didukung:
 *              LED:ON     → Nyalakan LED
 *              LED:OFF    → Matikan LED
 *              PWM:0-255  → Atur kecerahan (0=mati, 255=max)
 *              STATUS     → Tampilkan status saat ini
 *              HELP       → Daftar perintah
 *
 * RANGKAIAN: GPIO 2 → [220Ω] → LED → GND
 */

#include <Arduino.h>

const uint8_t PIN_LED = 2;
int currentPWM = 0;
bool ledOn = false;

void printHelp() {
  Serial.println("\n=== PERINTAH YANG TERSEDIA ===");
  Serial.println("  LED:ON     → Nyalakan LED (100%)");
  Serial.println("  LED:OFF    → Matikan LED");
  Serial.println("  PWM:0-255  → Atur kecerahan");
  Serial.println("  STATUS     → Status saat ini");
  Serial.println("  HELP       → Tampilkan bantuan ini");
  Serial.println("==============================\n");
}

void printStatus() {
  Serial.printf("[STATUS] LED=%s | PWM=%d | Brightness=%d%%\n",
                ledOn ? "ON" : "OFF", currentPWM, currentPWM * 100 / 255);
}

void prosesPerintah(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "LED:ON") {
    ledOn = true;
    currentPWM = 255;
    ledcWrite(PIN_LED, 255);
    Serial.println("[OK] LED ON ████████████");
  }
  else if (cmd == "LED:OFF") {
    ledOn = false;
    currentPWM = 0;
    ledcWrite(PIN_LED, 0);
    Serial.println("[OK] LED OFF ░░░░░░░░░░░");
  }
  else if (cmd.startsWith("PWM:")) {
    int val = cmd.substring(4).toInt();
    val = constrain(val, 0, 255);
    currentPWM = val;
    ledOn = (val > 0);
    ledcWrite(PIN_LED, val);
    Serial.printf("[OK] PWM = %d (%d%%)\n", val, val * 100 / 255);
  }
  else if (cmd == "STATUS") {
    printStatus();
  }
  else if (cmd == "HELP") {
    printHelp();
  }
  else {
    Serial.printf("[ERROR] Perintah tidak dikenal: '%s'. Ketik HELP.\n", cmd.c_str());
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);  // Timeout readString lebih cepat
  ledcAttach(PIN_LED, 5000, 8);

  delay(1000);
  Serial.println("ESP32 — Serial Command Parser");
  printHelp();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd.length() > 0) {
      Serial.printf("> %s\n", cmd.c_str());
      prosesPerintah(cmd);
    }
  }
}
```

---

### Praktikum 3.3 — Versi OLED: Serial Monitor + Display

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
 * PROGRAM  : Serial Command + OLED Display
 * DESKRIPSI: Kontrol LED via Serial + tampilkan status real-time di OLED.
 *            OLED menampilkan: status LED, PWM, perintah terakhir,
 *            jumlah perintah yang diproses, dan uptime.
 *
 * RANGKAIAN:
 *   GPIO 2  → [220Ω] → LED → GND
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const uint8_t PIN_LED = 2;
int currentPWM = 0;
bool ledOn = false;
String lastCmd = "(none)";
int cmdCount = 0;

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setCursor(0, 0);
  display.println("UART Command Monitor");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // LED Status
  display.setCursor(0, 13);
  display.printf("LED  : %s", ledOn ? "ON" : "OFF");

  // PWM + Bar
  display.setCursor(0, 23);
  display.printf("PWM  : %d (%d%%)", currentPWM, currentPWM * 100 / 255);
  display.drawRect(0, 33, 128, 6, SSD1306_WHITE);
  display.fillRect(1, 34, map(currentPWM, 0, 255, 0, 126), 4, SSD1306_WHITE);

  // Last command
  display.setCursor(0, 43);
  display.printf("Cmd  : %s", lastCmd.c_str());

  // Stats
  display.setCursor(0, 53);
  display.printf("#%d  Uptime:%lus", cmdCount, millis()/1000);

  display.display();
}

void prosesPerintah(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  lastCmd = cmd;
  cmdCount++;

  if (cmd == "LED:ON") {
    ledOn = true; currentPWM = 255;
    ledcWrite(PIN_LED, 255);
    Serial.println("[OK] LED ON");
  } else if (cmd == "LED:OFF") {
    ledOn = false; currentPWM = 0;
    ledcWrite(PIN_LED, 0);
    Serial.println("[OK] LED OFF");
  } else if (cmd.startsWith("PWM:")) {
    int val = constrain(cmd.substring(4).toInt(), 0, 255);
    currentPWM = val; ledOn = (val > 0);
    ledcWrite(PIN_LED, val);
    Serial.printf("[OK] PWM=%d\n", val);
  } else {
    Serial.printf("[ERR] '%s'?\n", cmd.c_str());
  }

  updateOLED();
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  ledcAttach(PIN_LED, 5000, 8);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }

  Serial.println("Serial + OLED Monitor");
  Serial.println("Perintah: LED:ON, LED:OFF, PWM:0-255");
  updateOLED();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    if (cmd.length() > 0) prosesPerintah(cmd);
  }

  // Update OLED setiap 1 detik (uptime)
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    updateOLED();
  }
}
```

---

## 4. Referensi

1. **Espressif.** *UART API Reference.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
2. **Arduino.** *Serial Reference.* [https://www.arduino.cc/reference/en/language/functions/communication/serial/](https://www.arduino.cc/reference/en/language/functions/communication/serial/)
3. **Random Nerd Tutorials.** *ESP32 Serial Communication.* [https://randomnerdtutorials.com/esp32-serial-communication-uart/](https://randomnerdtutorials.com/esp32-serial-communication-uart/)
