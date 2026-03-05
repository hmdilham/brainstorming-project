# Modul Pertemuan 9 — IOT Dasar
# Aktuator — Memberikan Aksi ke Dunia Fisik

---

## 1. Maksud dan Tujuan Materi

### Maksud
Aktuator adalah komponen yang mengubah sinyal listrik menjadi gerakan atau aksi fisik. Jika sensor adalah "mata" IoT, maka aktuator adalah "tangan" nya. Pertemuan ini membahas tiga aktuator utama: **Servo Motor** (kontrol sudut presisi), **Relay** (saklar beban tegangan tinggi), dan **Motor DC dengan L298N** (kontrol kecepatan dan arah).

### Tujuan Pembelajaran
1. **Mengontrol** sudut servo motor (0°–180°) menggunakan library `ESP32Servo`.
2. **Mengoperasikan** relay untuk mengendalikan beban AC/DC secara aman.
3. **Mengontrol** kecepatan dan arah motor DC menggunakan driver L298N.
4. **Membangun** sistem palang parkir otomatis (HC-SR04 + Servo).
5. **Menampilkan** status aktuator pada OLED display.

---

## 2. Teori Materi

### 2.1 Servo Motor SG90

Servo motor berputar ke sudut tertentu (0°-180°) berdasarkan lebar pulsa PWM.

| Parameter | Nilai |
|---|---|
| Tegangan | 4.8V – 6V (gunakan 5V dari VIN) |
| Sudut | 0° – 180° |
| Sinyal PWM | 50 Hz, pulsa 500µs (0°) – 2400µs (180°) |
| Torsi | 1.8 kg·cm |

```
  Pulsa 500µs  (duty ~2.5%)  → 0°    (kiri penuh)
  Pulsa 1500µs (duty ~7.5%)  → 90°   (tengah)
  Pulsa 2400µs (duty ~12%)   → 180°  (kanan penuh)
```

### 2.2 Relay Module

Relay adalah saklar elektromagnetik yang memungkinkan sinyal 3.3V ESP32 mengontrol beban bertegangan tinggi (AC 220V, DC 12V, dll.).

> ⚠️ **KESELAMATAN:** Relay yang mengontrol AC 220V melibatkan tegangan yang **BERBAHAYA dan bisa mematikan**. Selalu matikan sumber AC sebelum mengubah kabel.

**Koneksi Relay Module:**
```
  ESP32 GPIO → IN (kontrol)
  ESP32 GND  → GND
  5V (VIN)   → VCC relay module

  Relay bersifat ACTIVE LOW (IN=LOW → relay ON)
```

### 2.3 Motor DC + L298N

L298N adalah H-Bridge driver yang mengontrol arah dan kecepatan motor DC.

| Pin L298N | Fungsi | Hubungkan ke |
|---|---|---|
| ENA | PWM kecepatan Motor A | GPIO (PWM) |
| IN1 | Arah Motor A | GPIO |
| IN2 | Arah Motor A | GPIO |
| OUT1, OUT2 | Terminal Motor A | Motor DC |
| 12V / VCC | Power motor | Sumber 6-12V |
| 5V | Output regulator | (opsional, ke ESP32 VIN) |
| GND | Ground bersama | GND ESP32 + GND power |

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Servo Sweep + OLED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    madhephaestus/ESP32Servo@^3.0.5
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Servo Sweep + OLED Gauge
 * DESKRIPSI: Servo bergerak 0°→180°→0° secara bertahap.
 *            OLED menampilkan sudut dan visual gauge.
 *
 * RANGKAIAN:
 *   Servo Signal → GPIO 13
 *   Servo VCC    → 5V (VIN)
 *   Servo GND    → GND
 *   OLED SDA     → GPIO 21
 *   OLED SCL     → GPIO 22
 */

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo myServo;
const uint8_t PIN_SERVO = 13;

void tampilOLED(int sudut, const char* arah) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Servo Motor Control");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  // Sudut besar
  display.setTextSize(3);
  display.setCursor(10, 15);
  display.printf("%3d", sudut);
  display.setTextSize(1);
  display.setCursor(80, 25);
  display.printf("deg %s", arah);

  // Progress bar (0-180°)
  display.drawRect(0, 45, 128, 10, SSD1306_WHITE);
  int barW = map(sudut, 0, 180, 0, 126);
  display.fillRect(1, 46, barW, 8, SSD1306_WHITE);

  display.setCursor(0, 57);
  display.printf("0          90         180");

  display.display();
}

void setup() {
  Serial.begin(115200);
  myServo.attach(PIN_SERVO, 500, 2400); // min/max pulse µs

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }
  Serial.println("Servo Sweep + OLED");
}

void loop() {
  // Sweep 0 → 180
  for (int s = 0; s <= 180; s += 2) {
    myServo.write(s);
    tampilOLED(s, ">>");
    Serial.printf("Sudut: %3d° >>\n", s);
    delay(20);
  }
  delay(500);

  // Sweep 180 → 0
  for (int s = 180; s >= 0; s -= 2) {
    myServo.write(s);
    tampilOLED(s, "<<");
    delay(20);
  }
  delay(500);
}
```

---

### Praktikum 3.2 — Palang Parkir Otomatis (HC-SR04 + Servo)

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : Palang Parkir Otomatis
 * DESKRIPSI: HC-SR04 mendeteksi kendaraan (<30cm).
 *            Servo membuka palang (0°→90°), tahan 3 detik,
 *            lalu menutup (90°→0°). OLED menampilkan status.
 *
 * RANGKAIAN:
 *   GPIO 32 → HC-SR04 TRIG
 *   GPIO 33 → HC-SR04 ECHO (via voltage divider)
 *   GPIO 13 → Servo signal
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
Servo gate;

const uint8_t PIN_TRIG  = 32;
const uint8_t PIN_ECHO  = 33;
const uint8_t PIN_SERVO = 13;
const float   THRESHOLD = 30.0; // cm

enum State { CLOSED, OPENING, OPEN, CLOSING };
State state = CLOSED;
unsigned long stateTimer = 0;
int gateAngle = 0;
int carCount = 0;

float ukurJarak() {
  digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long d = pulseIn(PIN_ECHO, HIGH, 30000);
  return (d == 0) ? 999 : d * 0.0343 / 2.0;
}

void updateOLED(float jarak) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Palang Parkir Auto");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("Jarak: %.1f cm", jarak);

  display.setCursor(0, 23);
  const char* stateStr[] = {"TERTUTUP", "MEMBUKA..", "TERBUKA", "MENUTUP.."};
  display.printf("Palang: %s", stateStr[state]);

  display.setCursor(0, 33);
  display.printf("Sudut : %d deg", gateAngle);

  // Visual palang
  display.drawRect(0, 44, 128, 10, SSD1306_WHITE);
  int barW = map(gateAngle, 0, 90, 0, 126);
  display.fillRect(1, 45, barW, 8, SSD1306_WHITE);

  display.setCursor(0, 56);
  display.printf("Kendaraan: %d", carCount);

  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  gate.attach(PIN_SERVO, 500, 2400);
  gate.write(0);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }
  Serial.println("Palang Parkir Otomatis");
}

void loop() {
  float jarak = ukurJarak();

  switch (state) {
    case CLOSED:
      if (jarak < THRESHOLD) {
        state = OPENING;
        carCount++;
        Serial.printf("[#%d] Kendaraan terdeteksi! Membuka...\n", carCount);
      }
      break;

    case OPENING:
      gateAngle += 3;
      if (gateAngle >= 90) { gateAngle = 90; state = OPEN; stateTimer = millis(); }
      gate.write(gateAngle);
      break;

    case OPEN:
      if (millis() - stateTimer > 3000) { state = CLOSING; }
      break;

    case CLOSING:
      gateAngle -= 3;
      if (gateAngle <= 0) { gateAngle = 0; state = CLOSED; }
      gate.write(gateAngle);
      break;
  }

  updateOLED(jarak);
  delay(30);
}
```

---

## 4. Referensi

1. **ESP32Servo Library.** [https://github.com/madhephaestus/ESP32Servo](https://github.com/madhephaestus/ESP32Servo)
2. **Random Nerd Tutorials.** *ESP32 Servo Motor.* [https://randomnerdtutorials.com/esp32-servo-motor-web-server-arduino-ide/](https://randomnerdtutorials.com/esp32-servo-motor-web-server-arduino-ide/)
3. **Last Minute Engineers.** *L298N Motor Driver.* [https://lastminuteengineers.com/l298n-dc-stepper-driver-arduino-tutorial/](https://lastminuteengineers.com/l298n-dc-stepper-driver-arduino-tutorial/)
