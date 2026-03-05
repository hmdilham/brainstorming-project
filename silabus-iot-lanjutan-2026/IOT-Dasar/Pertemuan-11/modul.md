# Modul Pertemuan 11 — IOT Dasar
# Komunikasi Bluetooth Low Energy (BLE)

---

## 1. Maksud dan Tujuan Materi

### Maksud
Bluetooth Low Energy (BLE) adalah protokol komunikasi nirkabel jarak dekat dengan konsumsi daya sangat rendah. BLE ideal untuk wearable, sensor portabel, dan remote control. ESP32 memiliki BLE 4.2 built-in yang dapat berfungsi sebagai **Server** (mengirim data sensor) atau **Client** (menerima data/perintah).

### Tujuan Pembelajaran
1. **Membedakan** Bluetooth Classic vs BLE (arsitektur GATT, konsep Service/Characteristic).
2. **Membangun** ESP32 sebagai BLE Server yang mengirim data sensor.
3. **Menerima** perintah dari smartphone via BLE Write characteristic.
4. **Menggunakan** aplikasi nRF Connect untuk berinteraksi dengan BLE Server.
5. **Menampilkan** status koneksi BLE dan data pada OLED.

---

## 2. Teori Materi

### 2.1 Bluetooth Classic vs BLE

| Aspek | Bluetooth Classic | **BLE** |
|---|---|---|
| Konsumsi Daya | Tinggi (~100 mA) | **Sangat rendah (~15 mA saat TX)** |
| Throughput | Hingga 3 Mbps | ~1 Mbps |
| Range | ~10 m | ~10-30 m (indoor) |
| Koneksi | Stream (SPP) | **Event-driven (GATT)** |
| Ideal untuk | Audio streaming | **Sensor, remote, beacon** |

### 2.2 Arsitektur GATT (Generic Attribute Profile)

```
  ┌── BLE Server (ESP32) ──────────────────────────────────┐
  │                                                         │
  │  ┌── Service: "Environment" (UUID: 0x181A) ──────────┐ │
  │  │                                                     │ │
  │  │  ┌── Characteristic: "Temperature" ──────────────┐ │ │
  │  │  │   UUID: 0x2A6E                                 │ │ │
  │  │  │   Properties: READ, NOTIFY                     │ │ │
  │  │  │   Value: "28.5"                                │ │ │
  │  │  └───────────────────────────────────────────────┘ │ │
  │  │                                                     │ │
  │  │  ┌── Characteristic: "Humidity" ─────────────────┐ │ │
  │  │  │   UUID: 0x2A6F                                 │ │ │
  │  │  │   Properties: READ, NOTIFY                     │ │ │
  │  │  │   Value: "65.0"                                │ │ │
  │  │  └───────────────────────────────────────────────┘ │ │
  │  └────────────────────────────────────────────────────┘ │
  │                                                         │
  │  ┌── Service: "LED Control" (Custom UUID) ───────────┐ │
  │  │  ┌── Characteristic: "Switch" ──────────────────┐ │ │
  │  │  │   Properties: WRITE                           │ │ │
  │  │  │   Value: "ON" / "OFF"                         │ │ │
  │  │  └──────────────────────────────────────────────┘ │ │
  │  └────────────────────────────────────────────────────┘ │
  └─────────────────────────────────────────────────────────┘
```

### 2.3 UUID (Universally Unique Identifier)

- **Standard UUID** (16-bit, didefinisikan Bluetooth SIG): `0x181A` (Environmental Sensing)
- **Custom UUID** (128-bit): `4fafc201-1fb5-459e-8fcc-c5c9c331914b` — untuk service/characteristic custom

---

## 3. Panduan Praktikum

### Praktikum 3.1 — BLE Server: Sensor DHT22 + LED Control + OLED

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
 * PROGRAM  : BLE Server — Sensor + LED Control + OLED
 * DESKRIPSI: ESP32 sebagai BLE Server:
 *   - Service "Sensor": Notify suhu/kelembaban DHT22 setiap 2 detik
 *   - Service "LED": Write characteristic untuk kontrol LED
 *   - OLED menampilkan status koneksi, data sensor, LED
 *
 * RANGKAIAN:
 *   GPIO 4  → DHT22 DATA
 *   GPIO 2  → [220Ω] → LED → GND
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 *
 * TEST: Gunakan app nRF Connect (Android/iOS) untuk membaca dan write.
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(4, DHT22);

// BLE UUIDs
#define SERVICE_SENSOR_UUID   "181a0000-0000-1000-8000-00805f9b34fb"
#define CHAR_TEMP_UUID        "2a6e0000-0000-1000-8000-00805f9b34fb"
#define CHAR_HUM_UUID         "2a6f0000-0000-1000-8000-00805f9b34fb"
#define SERVICE_LED_UUID      "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_LED_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const uint8_t PIN_LED = 2;

BLEServer* pServer = nullptr;
BLECharacteristic* pTempChar = nullptr;
BLECharacteristic* pHumChar  = nullptr;

bool deviceConnected = false;
bool ledState = false;
unsigned long lastSend = 0;
int connCount = 0;

// Callback koneksi
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override {
    deviceConnected = true;
    connCount++;
    Serial.println("BLE Client terhubung!");
  }
  void onDisconnect(BLEServer* s) override {
    deviceConnected = false;
    Serial.println("BLE Client terputus. Re-advertising...");
    s->startAdvertising();
  }
};

// Callback write LED
class LEDCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pChar) override {
    String val = pChar->getValue().c_str();
    val.trim();
    val.toUpperCase();
    if (val == "ON" || val == "1") {
      ledState = true;
      digitalWrite(PIN_LED, HIGH);
      Serial.println("[BLE] LED ON");
    } else if (val == "OFF" || val == "0") {
      ledState = false;
      digitalWrite(PIN_LED, LOW);
      Serial.println("[BLE] LED OFF");
    }
  }
};

void updateOLED(float suhu, float hum) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.printf("BLE: %s", deviceConnected ? "CONNECTED" : "Advertising");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("Suhu  : %.1f C", suhu);
  display.setCursor(0, 23);
  display.printf("Lembab: %.1f %%", hum);
  display.setCursor(0, 33);
  display.printf("LED   : %s", ledState ? "ON" : "OFF");

  display.drawLine(0, 43, 128, 43, SSD1306_WHITE);
  display.setCursor(0, 47);
  display.printf("Device: ESP32-IoT");
  display.setCursor(0, 57);
  display.printf("Koneksi: %d total", connCount);

  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }

  // === BLE Setup ===
  BLEDevice::init("ESP32-IoT-Sensor");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Service Sensor
  BLEService* sensorService = pServer->createService(SERVICE_SENSOR_UUID);
  pTempChar = sensorService->createCharacteristic(
    CHAR_TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTempChar->addDescriptor(new BLE2902());
  pHumChar = sensorService->createCharacteristic(
    CHAR_HUM_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pHumChar->addDescriptor(new BLE2902());
  sensorService->start();

  // Service LED
  BLEService* ledService = pServer->createService(SERVICE_LED_UUID);
  BLECharacteristic* pLedChar = ledService->createCharacteristic(
    CHAR_LED_UUID, BLECharacteristic::PROPERTY_WRITE);
  pLedChar->setCallbacks(new LEDCallbacks());
  ledService->start();

  // Advertising
  BLEAdvertising* pAdv = BLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_SENSOR_UUID);
  pAdv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("BLE Server ready! Nama: ESP32-IoT-Sensor");
}

void loop() {
  float suhu = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (deviceConnected && (millis() - lastSend > 2000)) {
    lastSend = millis();
    if (!isnan(suhu)) {
      char buf[10];
      snprintf(buf, sizeof(buf), "%.1f", suhu);
      pTempChar->setValue(buf);
      pTempChar->notify();

      snprintf(buf, sizeof(buf), "%.1f", hum);
      pHumChar->setValue(buf);
      pHumChar->notify();

      Serial.printf("[BLE TX] T=%.1f H=%.1f\n", suhu, hum);
    }
  }

  updateOLED(isnan(suhu) ? 0 : suhu, isnan(hum) ? 0 : hum);
  delay(500);
}
```

---

## 4. Referensi

1. **Espressif.** *BLE API.* [https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ble.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ble.html)
2. **Random Nerd Tutorials.** *ESP32 BLE Server.* [https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/](https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/)
3. **nRF Connect App.** [https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile)
