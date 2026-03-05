# Modul Pertemuan 2 — IOT Dasar
# Instalasi PlatformIO & Proyek Pertama (Blink)

---

## 1. Maksud dan Tujuan Materi

### Maksud
Sebelum membangun sistem IoT, mahasiswa memerlukan lingkungan pengembangan yang handal dan profesional. Pertemuan ini memandu instalasi **PlatformIO** (IDE utama berbasis VS Code) dan menjalankan program pertama: menyalakan LED internal ESP32 (Blink). Arduino IDE dibahas singkat sebagai alternatif.

### Tujuan Pembelajaran
1. **Menginstal** Visual Studio Code dan ekstensi PlatformIO IDE.
2. **Membuat** proyek PlatformIO baru untuk board ESP32 DevKit.
3. **Memahami** struktur proyek PlatformIO: `platformio.ini`, `src/`, `lib/`, `include/`.
4. **Menjelaskan** struktur dasar program Arduino: `setup()` dan `loop()`.
5. **Mengupload** program pertama (Blink) ke ESP32 dan memverifikasi hasilnya.
6. **Menggunakan** Serial Monitor untuk debugging dan output data.

---

## 2. Teori Materi

### 2.1 Mengapa PlatformIO?

| Fitur | Arduino IDE | **PlatformIO (VS Code)** |
|---|---|---|
| Code Completion | Terbatas | **✅ IntelliSense penuh** |
| Multi-file Project | Manual | **✅ Otomatis (src/, lib/, include/)** |
| Library Management | Library Manager (global) | **✅ Per-project (`lib_deps`)** |
| Build System | Sederhana | **✅ CMake-based, cepat** |
| Debugging | Terbatas | **✅ GDB/JTAG, breakpoints** |
| Multi-Board | Pilih satu di UI | **✅ Banyak env di `platformio.ini`** |
| Version Control | Manual | **✅ Git-friendly** |
| Terminal Terintegrasi | ❌ | **✅ Bawaan VS Code** |

### 2.2 Struktur Proyek PlatformIO

```
my-iot-project/
├── platformio.ini          ← Konfigurasi utama (board, framework, library)
├── src/
│   └── main.cpp            ← Kode program utama (setara .ino di Arduino)
├── lib/
│   └── README              ← Library lokal proyek (opsional)
├── include/
│   └── README              ← Header files (.h) proyek
├── test/                   ← Unit test (opsional)
└── .pio/                   ← Build cache (otomatis, jangan edit)
```

### 2.3 File `platformio.ini`

File ini adalah jantung konfigurasi proyek. Mendefinisikan board, framework, kecepatan serial, dan dependensi library.

```ini
; platformio.ini — Konfigurasi proyek PlatformIO

[env:esp32dev]
platform = espressif32        ; Platform Espressif ESP32
board = esp32dev              ; Board: ESP32 DevKit v1 (generic)
framework = arduino           ; Framework: Arduino (bukan ESP-IDF)
monitor_speed = 115200        ; Baud rate Serial Monitor
upload_speed = 921600         ; Kecepatan upload (opsional, default 460800)

; Dependensi library (ditambahkan sesuai kebutuhan)
; lib_deps =
;     adafruit/DHT sensor library@^1.4.6
```

**Perintah PlatformIO CLI yang penting:**

| Perintah | Fungsi |
|---|---|
| `pio run` | Compile/build project |
| `pio run -t upload` | Compile + upload ke board |
| `pio device monitor` | Buka Serial Monitor |
| `pio run -t upload && pio device monitor` | Upload lalu langsung monitor |
| `pio lib install "DHT sensor library"` | Install library |
| `pio boards` | Daftar semua board yang didukung |

### 2.4 Perbedaan Kode Arduino IDE vs PlatformIO

```cpp
// === Arduino IDE (.ino) ===
// Tidak perlu #include <Arduino.h>
// File extension: .ino

void setup() {
  pinMode(2, OUTPUT);
}
void loop() {
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
```

```cpp
// === PlatformIO (src/main.cpp) ===
// WAJIB: #include <Arduino.h>
// File extension: .cpp

#include <Arduino.h>      // ← Wajib di PlatformIO!

void setup() {
  pinMode(2, OUTPUT);
}
void loop() {
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
```

> **Perbedaan utama:** Di PlatformIO, `#include <Arduino.h>` **wajib** ditulis di baris pertama. Di Arduino IDE, ini otomatis ditambahkan oleh preprocessor.

### 2.5 Struktur Program Arduino

```
┌─────────────────────────────────────┐
│  #include <Arduino.h>               │  ← Header & deklarasi
│  #include <Library.h>               │
│  int variabel = 0;                  │
├─────────────────────────────────────┤
│  void setup() {                     │  ← Dijalankan SEKALI saat boot
│    // Inisialisasi                  │     - Konfigurasi pin (INPUT/OUTPUT)
│    // Serial.begin()                │     - Memulai komunikasi serial
│    // WiFi.begin()                  │     - Koneksi WiFi
│  }                                  │
├─────────────────────────────────────┤
│  void loop() {                      │  ← Dijalankan BERULANG selamanya
│    // Baca sensor                   │     - Membaca input
│    // Proses data                   │     - Memproses data
│    // Tulis aktuator                │     - Mengontrol output
│  }                                  │
└─────────────────────────────────────┘
```

### 2.6 Tipe Data Dasar C/C++ untuk Arduino

| Tipe | Ukuran | Range | Contoh |
|---|---|---|---|
| `bool` | 1 byte | `true` / `false` | `bool ledState = false;` |
| `int` | 4 byte (ESP32) | -2.1B ~ +2.1B | `int counter = 0;` |
| `float` | 4 byte | ±3.4E38 | `float suhu = 28.5;` |
| `double` | 8 byte | ±1.7E308 | `double presisi = 3.14159;` |
| `char` | 1 byte | -128 ~ 127 | `char karakter = 'A';` |
| `String` | variabel | Teks dinamis | `String nama = "ESP32";` |
| `uint8_t` | 1 byte | 0 ~ 255 | `uint8_t pin = 2;` |
| `uint32_t` | 4 byte | 0 ~ 4.2B | `uint32_t waktu = millis();` |
| `unsigned long` | 4 byte | 0 ~ 4.2B | `unsigned long prev = 0;` |

### 2.7 Instalasi Driver USB-to-UART

ESP32 DevKit menggunakan chip USB-to-Serial untuk komunikasi dengan komputer:

| Chip | Driver | Link |
|---|---|---|
| **CP2102** (Silicon Labs) | CP210x VCP Driver | [silabs.com/developers/usb-to-uart-bridge-vcp-drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |
| **CH340** (WCH) | CH340 Driver | [wch-ic.com/downloads/CH341SER_EXE.html](https://www.wch-ic.com/downloads/CH341SER_EXE.html) |

> **Linux:** Biasanya sudah include driver. Jika permission denied: `sudo usermod -aG dialout $USER` lalu re-login.
> **macOS:** `brew install --cask silicon-labs-vcp-driver` atau unduh manual.

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Instalasi PlatformIO

**Langkah-langkah:**

1. **Instal Visual Studio Code:**
   - Unduh dari [https://code.visualstudio.com](https://code.visualstudio.com)
   - Install sesuai OS (Windows/macOS/Linux)

2. **Instal Ekstensi PlatformIO IDE:**
   - Buka VS Code → klik ikon Extensions (Ctrl+Shift+X)
   - Cari **"PlatformIO IDE"** → klik **Install**
   - Tunggu hingga instalasi selesai (~2-5 menit, mengunduh Python toolchain)
   - Restart VS Code jika diminta

3. **Buat Proyek Baru:**
   - Klik ikon **PlatformIO** di sidebar kiri (ikon alien/semut)
   - Klik **"New Project"**
   - **Name:** `01-blink`
   - **Board:** `Espressif ESP32 Dev Module` (cari "esp32dev")
   - **Framework:** `Arduino`
   - **Location:** pilih folder kerja Anda
   - Klik **Finish** — tunggu PlatformIO mengunduh toolchain ESP32

4. **Verifikasi Struktur:**
   - Pastikan ada file `platformio.ini` dan folder `src/` dengan `main.cpp`

---

### Praktikum 3.2 — Program Blink (LED Internal)

**Kebutuhan:** ESP32 DevKit v1, Kabel USB

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

**`src/main.cpp` — Blink LED Internal:**
```cpp
/*
 * PROGRAM  : Blink — LED Internal ESP32
 * PLATFORM : PlatformIO + ESP32 DevKit v1
 * DESKRIPSI: LED built-in (GPIO 2) berkedip setiap 500ms.
 *            Program "Hello World" pertama untuk ESP32.
 *
 * CATATAN  : LED built-in pada kebanyakan ESP32 DevKit ada di GPIO 2.
 *            Beberapa board menggunakan GPIO yang berbeda.
 *            Konstanta LED_BUILTIN biasanya sudah didefinisikan.
 */

#include <Arduino.h>

// Pin LED built-in (umumnya GPIO 2 pada ESP32 DevKit v1)
const uint8_t PIN_LED = LED_BUILTIN;  // atau langsung: 2

void setup() {
  // Inisialisasi Serial Monitor untuk debugging
  Serial.begin(115200);
  delay(1000);  // Tunggu Serial siap

  Serial.println("============================");
  Serial.println("  ESP32 Blink — PlatformIO");
  Serial.println("============================");
  Serial.printf("LED Pin: GPIO %d\n", PIN_LED);

  // Konfigurasi pin LED sebagai OUTPUT
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  // Nyalakan LED
  digitalWrite(PIN_LED, HIGH);
  Serial.println("[LED] ON  █████");
  delay(500);

  // Matikan LED
  digitalWrite(PIN_LED, LOW);
  Serial.println("[LED] OFF ░░░░░");
  delay(500);
}
```

**Cara Upload & Monitor:**
1. Hubungkan ESP32 ke komputer via USB
2. Di VS Code, klik  **→** (tombol upload) di status bar bawah, atau tekan `Ctrl+Alt+U`
3. Tunggu proses compile + upload ("SUCCESS" muncul di terminal)
4. Klik ikon **🔌** (Serial Monitor) di status bar, atau tekan `Ctrl+Alt+S`
5. Amati LED built-in berkedip dan pesan muncul di Serial Monitor

---

### Praktikum 3.3 — Blink Pattern: 3× Cepat + Jeda

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Blink Pattern — 3x Cepat + Jeda
 * DESKRIPSI: LED berkedip 3 kali cepat (100ms), berhenti 1 detik, ulangi.
 *            Melatih penggunaan loop for dan delay.
 */

#include <Arduino.h>

const uint8_t PIN_LED = LED_BUILTIN;

void blinkCepat(int jumlah, int durasiMs) {
  for (int i = 0; i < jumlah; i++) {
    digitalWrite(PIN_LED, HIGH);
    delay(durasiMs);
    digitalWrite(PIN_LED, LOW);
    delay(durasiMs);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  Serial.println("Blink Pattern: 3x cepat + jeda 1 detik");
}

void loop() {
  Serial.println(">>> 3x Blink cepat");
  blinkCepat(3, 100);  // 3 kali kedip, masing-masing 100ms

  Serial.println("--- Jeda 1 detik ---");
  delay(1000);          // Jeda 1 detik
}
```

---

### Praktikum 3.4 — Serial Monitor: Hello World + Counter

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Serial Hello World + Counter
 * DESKRIPSI: Mendemonstrasikan penggunaan Serial Monitor untuk debugging.
 *            Menampilkan pesan boot, info chip, dan counter naik.
 */

#include <Arduino.h>

uint32_t counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Informasi chip ESP32
  Serial.println("╔══════════════════════════════════╗");
  Serial.println("║    ESP32 — Serial Monitor Demo   ║");
  Serial.println("╚══════════════════════════════════╝");
  Serial.printf("Chip Model   : %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Cores    : %d\n", ESP.getChipCores());
  Serial.printf("CPU Freq     : %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size   : %d KB\n", ESP.getFlashChipSize() / 1024);
  Serial.printf("Free Heap    : %d bytes\n", ESP.getFreeHeap());
  Serial.printf("SDK Version  : %s\n", ESP.getSdkVersion());
  Serial.println();
}

void loop() {
  counter++;
  unsigned long uptime = millis() / 1000;

  Serial.printf("[%6lu s] Counter: %lu | Free Heap: %d bytes\n",
                uptime, counter, ESP.getFreeHeap());

  delay(1000);
}
```

---

### Praktikum 3.5 — Arduino IDE sebagai Alternatif (Singkat)

Jika karena suatu alasan PlatformIO tidak bisa diinstal:

1. Unduh Arduino IDE 2.x dari [arduino.cc/en/software](https://www.arduino.cc/en/software)
2. Buka **File → Preferences** → tambahkan URL berikut di "Additional Board Manager URLs":
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Buka **Tools → Board → Boards Manager** → cari "esp32" → instal **"esp32 by Espressif Systems"**
4. Pilih **Tools → Board → ESP32 Dev Module**
5. Pilih port COM yang sesuai
6. Copy kode Blink (tanpa `#include <Arduino.h>`) → Upload

> **Catatan:** Seluruh modul dalam kursus ini menggunakan format **PlatformIO** (`platformio.ini` + `src/main.cpp`). Jika Anda menggunakan Arduino IDE, hapus baris `#include <Arduino.h>` dan simpan file sebagai `.ino`.

---

## 4. Referensi

1. **PlatformIO.** *Getting Started.* [https://docs.platformio.org/en/latest/tutorials/espressif32/arduino_debugging_unit_testing.html](https://docs.platformio.org/en/latest/tutorials/espressif32/arduino_debugging_unit_testing.html)
2. **PlatformIO.** *platformio.ini Reference.* [https://docs.platformio.org/en/latest/projectconf/index.html](https://docs.platformio.org/en/latest/projectconf/index.html)
3. **Espressif.** *Arduino-ESP32 Installation.* [https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
4. **VS Code.** *Download.* [https://code.visualstudio.com](https://code.visualstudio.com)
