# Modul Pertemuan 6 — IOT Lanjutan
# HTTP Client — POST Request & Pengiriman Data ke Server

---

## 1. Maksud dan Tujuan Materi

### Maksud
Mengirim data sensor dari ESP32 ke server menggunakan HTTP POST. Mahasiswa belajar membuat JSON payload, mengirim ke webhook.site untuk verifikasi, dan mengirim ke ThingSpeak untuk visualisasi grafik.

### Tujuan Pembelajaran
1. **Mengirim** HTTP POST dengan JSON payload dari ESP32.
2. **Memverifikasi** data di webhook.site/ThingSpeak.
3. **Mengimplementasikan** API Key authentication.

---

## 2. Praktikum — POST Sensor Data + OLED Status

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
    bblanchon/ArduinoJson@^7.0.0
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(4, DHT22);

const char* SSID = "WIFI_ANDA";
const char* PASS = "PASSWORD";
const char* WEBHOOK_URL = "https://webhook.site/MASUKKAN-UUID-ANDA";
// Atau ThingSpeak: "https://api.thingspeak.com/update"
// const char* THINGSPEAK_KEY = "API_KEY_ANDA";

int postCount = 0;
int successCount = 0;

void postData(float suhu, float hum) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(WEBHOOK_URL);
  http.addHeader("Content-Type", "application/json");
  // http.addHeader("X-API-Key", "your-key");  // Opsional

  JsonDocument doc;
  doc["device"] = "ESP32-IoT";
  doc["suhu"] = round(suhu * 10) / 10.0;
  doc["kelembaban"] = round(hum * 10) / 10.0;
  doc["uptime_s"] = millis() / 1000;

  String payload;
  serializeJson(doc, payload);

  postCount++;
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    successCount++;
    Serial.printf("[#%d] POST %d: %s\n", postCount, httpCode, payload.c_str());
  } else {
    Serial.printf("[#%d] POST FAIL: %s\n", postCount, http.errorToString(httpCode).c_str());
  }
  http.end();
}

void updateOLED(float suhu, float hum) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("HTTP POST Monitor");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
  display.setCursor(0, 13); display.printf("Suhu : %.1f C", suhu);
  display.setCursor(0, 23); display.printf("Hum  : %.1f %%", hum);
  display.setCursor(0, 35); display.printf("POST : %d/%d OK", successCount, postCount);
  display.setCursor(0, 45); display.printf("WiFi : %s", WiFi.status()==WL_CONNECTED?"OK":"DISC");
  display.setCursor(0, 55); display.printf("Up: %lu s", millis()/1000);
  display.display();
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED gagal!");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
  static unsigned long last = 0;
  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();

  if (millis() - last > 15000 && !isnan(suhu)) {
    last = millis();
    postData(suhu, hum);
  }

  updateOLED(isnan(suhu)?0:suhu, isnan(hum)?0:hum);
  delay(500);
}
```

---

## 3. Referensi

1. **webhook.site.** [https://webhook.site](https://webhook.site)
2. **ThingSpeak.** [https://thingspeak.com/](https://thingspeak.com/)
