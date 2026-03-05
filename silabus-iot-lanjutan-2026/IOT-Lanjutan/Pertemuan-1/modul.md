# Modul Pertemuan 1 — IOT Lanjutan
# Komunikasi Antar ESP32 — UART (ESP32 ↔ ESP32)

---

## 1. Maksud dan Tujuan Materi

### Maksud
Di IOT Dasar, UART digunakan untuk komunikasi ESP32 ↔ Serial Monitor atau ESP32 ↔ modul GPS. Kini, protokol yang **sama** digunakan untuk komunikasi **langsung antara dua ESP32** — pembagian tugas: satu ESP32 membaca sensor, ESP32 lain mengontrol aktuator.

### Tujuan Pembelajaran
1. **Membangun** komunikasi UART dua arah antara dua ESP32 (cross TX↔RX).
2. **Merancang** protokol paket data sederhana: delimiter, format `KEY:VALUE`, ACK.
3. **Mengimplementasikan** error handling: timeout, data corrupt.
4. **Menampilkan** data yang diterima pada OLED di sisi penerima.

---

## 2. Teori Materi

### 2.1 Konteks: Mengapa Komunikasi Antar-ESP32?

| Skenario | Tanpa Komunikasi Antar | **Dengan Komunikasi Antar** |
|---|---|---|
| Robot sensor | 1 ESP32 kewalahan | ESP32-A: sensor + AI, ESP32-B: motor |
| Smart Home | 1 ESP32 terbatas GPIO | ESP32 per ruangan, 1 gateway |
| Data logger | Jarak sensor terbatas | ESP32 di lapangan + ESP32 di base |

### 2.2 Pengkabelan UART ESP32-to-ESP32

```
  ESP32-A (Sensor)              ESP32-B (Aktuator)
  ┌──────────────┐              ┌──────────────┐
  │         TX2  ├──────────────┤ RX2          │
  │  (GPIO 17)   │    Cross     │  (GPIO 16)   │
  │              │   Connect    │              │
  │         RX2  ├──────────────┤ TX2          │
  │  (GPIO 16)   │              │  (GPIO 17)   │
  │              │              │              │
  │         GND  ├──────────────┤ GND          │  ← GND WAJIB dihubungkan!
  └──────────────┘              └──────────────┘

  ⚠️ JANGAN hubungkan UART0 (GPIO 1/3) — itu untuk Serial Monitor!
  Gunakan UART2 (GPIO 16/17) untuk komunikasi antar-ESP32.
```

### 2.3 Protokol Paket Data Sederhana

```
  Format: "KEY:VALUE\n"
  Contoh: "T:28.5\n", "H:65.2\n", "ACK\n"

  Aturan:
  - Setiap paket diakhiri newline '\n'
  - Key dan value dipisah ':'
  - Sender mengirim data, receiver membalas "ACK\n"
  - Jika tidak ada ACK dalam 1 detik → kirim ulang
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — ESP32-A (Sensor Sender)

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
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : ESP32-A — Sensor Sender (UART2)
 * DESKRIPSI: Membaca DHT22, mengirim data via UART2 ke ESP32-B
 *            dalam format "T:28.5\n" dan "H:65.2\n".
 *            Menunggu ACK dari ESP32-B.
 *
 * RANGKAIAN:
 *   GPIO 4  → DHT22 DATA
 *   GPIO 17 (TX2) → ESP32-B GPIO 16 (RX2)
 *   GPIO 16 (RX2) → ESP32-B GPIO 17 (TX2)
 *   GND → ESP32-B GND
 */

#include <Arduino.h>
#include <DHT.h>

DHT dht(4, DHT22);

unsigned long lastSend = 0;
int sendCount = 0;
int ackCount = 0;

bool waitForACK(unsigned long timeoutMs) {
  unsigned long start = millis();
  String response = "";
  while (millis() - start < timeoutMs) {
    if (Serial2.available()) {
      char c = Serial2.read();
      if (c == '\n') {
        response.trim();
        if (response == "ACK") return true;
        response = "";
      } else {
        response += c;
      }
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);   // USB → Serial Monitor
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // UART2: RX=16, TX=17
  dht.begin();

  Serial.println("═══════════════════════════════");
  Serial.println("  ESP32-A: UART Sensor Sender");
  Serial.println("═══════════════════════════════");
}

void loop() {
  if (millis() - lastSend < 3000) return;
  lastSend = millis();

  float suhu = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (isnan(suhu) || isnan(hum)) {
    Serial.println("[ERR] DHT22 gagal baca!");
    return;
  }

  sendCount++;

  // Kirim suhu
  String msg = "T:" + String(suhu, 1) + "\n";
  Serial2.print(msg);
  Serial.printf("[#%d] TX: %s", sendCount, msg.c_str());

  // Kirim kelembaban
  msg = "H:" + String(hum, 1) + "\n";
  Serial2.print(msg);
  Serial.printf("      TX: %s", msg.c_str());

  // Tunggu ACK
  if (waitForACK(1000)) {
    ackCount++;
    Serial.printf("      RX: ACK ✅ (%d/%d)\n", ackCount, sendCount);
  } else {
    Serial.printf("      RX: TIMEOUT ❌ (%d/%d)\n", ackCount, sendCount);
  }
}
```

---

### Praktikum 3.2 — ESP32-B (Aktuator Receiver + OLED)

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
 * PROGRAM  : ESP32-B — Aktuator Receiver + OLED
 * DESKRIPSI: Menerima data dari ESP32-A via UART2, parse,
 *            mengontrol relay jika suhu >30°C, mengirim ACK,
 *            menampilkan semua data di OLED.
 *
 * RANGKAIAN:
 *   GPIO 16 (RX2) → ESP32-A GPIO 17 (TX2)
 *   GPIO 17 (TX2) → ESP32-A GPIO 16 (RX2)
 *   GND → ESP32-A GND
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
const float   THRESHOLD = 30.0;

float rxSuhu = 0, rxHum = 0;
int rxCount = 0;
bool relayState = false;
String lastRaw = "";

void prosesData(String line) {
  line.trim();
  if (line.length() == 0) return;

  lastRaw = line;
  int sep = line.indexOf(':');
  if (sep < 0) return;

  String key = line.substring(0, sep);
  float val = line.substring(sep + 1).toFloat();

  if (key == "T") {
    rxSuhu = val;
    rxCount++;

    // Kontrol relay berdasarkan suhu
    relayState = (rxSuhu > THRESHOLD);
    digitalWrite(PIN_LED, relayState);

    // Kirim ACK
    Serial2.print("ACK\n");
    Serial.printf("[#%d] RX: T=%.1f H=%.1f Relay=%s\n",
                  rxCount, rxSuhu, rxHum, relayState ? "ON" : "OFF");
  }
  else if (key == "H") {
    rxHum = val;
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("UART Receiver");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("Suhu  : %.1f C", rxSuhu);
  display.setCursor(0, 23);
  display.printf("Hum   : %.1f %%", rxHum);
  display.setCursor(0, 33);
  display.printf("Relay : %s (>%.0fC)", relayState ? "ON" : "OFF", THRESHOLD);
  display.setCursor(0, 43);
  display.printf("Paket : %d", rxCount);
  display.setCursor(0, 55);
  display.printf("Raw: %s", lastRaw.c_str());

  display.display();
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  pinMode(PIN_LED, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }
  Serial.println("ESP32-B: UART Receiver + OLED");
}

void loop() {
  // Baca data dari UART2
  static String buffer = "";
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      prosesData(buffer);
      buffer = "";
    } else {
      buffer += c;
    }
  }

  static unsigned long lastOLED = 0;
  if (millis() - lastOLED > 500) {
    lastOLED = millis();
    updateOLED();
  }
}
```

---

## 4. Referensi

1. **Espressif.** *UART API.* [https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/uart.html)
2. **Random Nerd Tutorials.** *ESP32 UART Communication.* [https://randomnerdtutorials.com/esp32-serial-communication-uart/](https://randomnerdtutorials.com/esp32-serial-communication-uart/)
