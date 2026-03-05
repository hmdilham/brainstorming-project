# Modul Pertemuan 2 — IOT Lanjutan
# Komunikasi Antar ESP32 — I2C Master/Slave

---

## 1. Maksud dan Tujuan Materi

### Maksud
Di IOT Dasar, I2C digunakan untuk ESP32 (Master) ↔ sensor/OLED (Slave). Kini, **ESP32 lain berfungsi sebagai Slave** — dua mikrokontroler berkomunikasi dalam satu bus I2C. Master meminta data, Slave merespons dan menerima perintah.

### Tujuan Pembelajaran
1. **Mengonfigurasi** ESP32 sebagai I2C Slave dengan alamat kustom.
2. **Mengimplementasikan** request-response: Master request → Slave respond data sensor.
3. **Mengirim** perintah dari Master ke Slave untuk kontrol aktuator.
4. **Menggabungkan** ESP32 Slave + OLED pada satu bus I2C.

---

## 2. Teori Materi

### 2.1 I2C Slave Mode pada ESP32

```
  I2C Bus (SDA + SCL)
  ──────────┬──────────────┬──────────────
            │              │
     ┌──────┴────┐  ┌──────┴────┐  ┌──────┴────┐
     │  MASTER   │  │  SLAVE    │  │  SLAVE    │
     │  ESP32-B  │  │  ESP32-A  │  │  OLED     │
     │           │  │  addr:0x08│  │  addr:0x3C│
     └───────────┘  └───────────┘  └───────────┘

  Master request ke 0x08 → ESP32-A kirim data sensor
  Master write ke 0x08   → ESP32-A terima perintah
  Master write ke 0x3C   → OLED menampilkan data
```

### 2.2 API Wire.h untuk Slave Mode

```cpp
// SLAVE Setup
Wire.begin(0x08);                // Mulai sebagai slave alamat 0x08
Wire.onReceive(receiveHandler);  // Callback: data masuk dari Master
Wire.onRequest(requestHandler);  // Callback: Master minta data

void receiveHandler(int numBytes) {
  while (Wire.available()) {
    uint8_t cmd = Wire.read();
    // Proses perintah dari Master
  }
}

void requestHandler() {
  // Master meminta data, kirim response
  Wire.write(dataBuffer, length);
}
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — ESP32-A (Slave: Sensor + Aktuator)

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
 * PROGRAM  : ESP32-A — I2C Slave (Sensor + LED)
 * DESKRIPSI: Berfungsi sebagai perangkat I2C alamat 0x08.
 *   - onRequest: kirim 4 byte (suhu int16 + hum int16)
 *   - onReceive: terima perintah LED (0x01=ON, 0x00=OFF)
 *
 * RANGKAIAN:
 *   GPIO 21 (SDA) → ESP32-B GPIO 21 (SDA)
 *   GPIO 22 (SCL) → ESP32-B GPIO 22 (SCL)
 *   GND → ESP32-B GND
 *   GPIO 4 → DHT22 DATA
 *   GPIO 2 → LED
 */

#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>

const uint8_t I2C_ADDR = 0x08;
const uint8_t PIN_LED = 2;
DHT dht(4, DHT22);

volatile bool ledCmd = false;
float lastSuhu = 0, lastHum = 0;
int reqCount = 0;

// Master meminta data sensor
void onRequest() {
  reqCount++;
  // Kirim suhu × 10 dan hum × 10 sebagai int16 (4 byte total)
  int16_t t = (int16_t)(lastSuhu * 10);
  int16_t h = (int16_t)(lastHum * 10);
  uint8_t buf[4];
  buf[0] = (t >> 8) & 0xFF; buf[1] = t & 0xFF;
  buf[2] = (h >> 8) & 0xFF; buf[3] = h & 0xFF;
  Wire.write(buf, 4);
}

// Master mengirim perintah
void onReceive(int numBytes) {
  while (Wire.available()) {
    uint8_t cmd = Wire.read();
    ledCmd = (cmd == 0x01);
    digitalWrite(PIN_LED, ledCmd);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  Wire.begin(I2C_ADDR);     // Mulai sebagai Slave 0x08
  Wire.onRequest(onRequest);
  Wire.onReceive(onReceive);

  Serial.printf("ESP32-A: I2C Slave (0x%02X)\n", I2C_ADDR);
}

void loop() {
  // Update sensor data
  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    float s = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(s)) { lastSuhu = s; lastHum = h; }

    Serial.printf("[Slave] T=%.1f H=%.1f LED=%s Req=%d\n",
                  lastSuhu, lastHum, ledCmd?"ON":"OFF", reqCount);
  }
}
```

---

### Praktikum 3.2 — ESP32-B (Master: Request + Display + OLED)

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
 * PROGRAM  : ESP32-B — I2C Master (Request + Display)
 * DESKRIPSI: Meminta data sensor dari ESP32-A (Slave 0x08)
 *            setiap 3 detik. Jika suhu >30°C, kirim perintah
 *            LED ON (0x01) ke Slave. Tampilkan di OLED.
 *
 * RANGKAIAN:
 *   GPIO 21 (SDA) → ESP32-A GPIO 21 + OLED SDA
 *   GPIO 22 (SCL) → ESP32-A GPIO 22 + OLED SCL
 *   GND → ESP32-A GND + OLED GND
 *   3.3V → OLED VCC
 *   Pull-up: 4.7kΩ SDA→3.3V, 4.7kΩ SCL→3.3V
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const uint8_t SLAVE_ADDR = 0x08;
const float THRESHOLD = 30.0;

float rxSuhu = 0, rxHum = 0;
bool ledCmd = false;
int pollCount = 0;
bool slaveOK = false;

void requestSensor() {
  pollCount++;
  uint8_t n = Wire.requestFrom(SLAVE_ADDR, (uint8_t)4);

  if (n == 4) {
    slaveOK = true;
    int16_t t = (Wire.read() << 8) | Wire.read();
    int16_t h = (Wire.read() << 8) | Wire.read();
    rxSuhu = t / 10.0;
    rxHum  = h / 10.0;

    // Kontrol LED berdasarkan suhu
    bool newCmd = (rxSuhu > THRESHOLD);
    if (newCmd != ledCmd) {
      ledCmd = newCmd;
      Wire.beginTransmission(SLAVE_ADDR);
      Wire.write(ledCmd ? 0x01 : 0x00);
      Wire.endTransmission();
    }

    Serial.printf("[#%d] T=%.1f H=%.1f LED=%s\n",
                  pollCount, rxSuhu, rxHum, ledCmd?"ON":"OFF");
  } else {
    slaveOK = false;
    Serial.printf("[#%d] Slave tidak merespons!\n", pollCount);
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("I2C Master Monitor");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("Slave 0x%02X: %s", SLAVE_ADDR, slaveOK?"OK":"FAIL");

  display.setTextSize(2);
  display.setCursor(0, 25);
  display.printf("%.1fC", rxSuhu);
  display.setTextSize(1);
  display.setCursor(80, 30);
  display.printf("%.0f%%", rxHum);

  display.setTextSize(1);
  display.setCursor(0, 43);
  display.printf("LED Slave: %s", ledCmd ? "ON (>30C)" : "OFF");
  display.setCursor(0, 55);
  display.printf("Poll #%d", pollCount);

  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }
  Serial.println("ESP32-B: I2C Master");
}

void loop() {
  static unsigned long lastPoll = 0;
  if (millis() - lastPoll >= 3000) {
    lastPoll = millis();
    requestSensor();
  }

  static unsigned long lastOLED = 0;
  if (millis() - lastOLED >= 500) {
    lastOLED = millis();
    updateOLED();
  }
}
```

---

## 4. Referensi

1. **Arduino.** *Wire Library (Slave).* [https://www.arduino.cc/reference/en/language/functions/communication/wire/](https://www.arduino.cc/reference/en/language/functions/communication/wire/)
2. **Random Nerd Tutorials.** *ESP32 I2C Master-Slave.* [https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/](https://randomnerdtutorials.com/esp32-i2c-communication-arduino-ide/)
