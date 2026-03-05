# Modul Pertemuan 5 — IOT Lanjutan
# HTTP Client — GET Request & Konsumsi REST API

---

## 1. Maksud dan Tujuan Materi

### Maksud
ESP32 sebagai HTTP Client mengonsumsi data dari REST API publik di internet — mengambil data cuaca, waktu, atau layanan web lainnya. Pertemuan ini mencakup HTTP GET, parsing JSON response, error handling, dan tampilan data di OLED.

### Tujuan Pembelajaran
1. **Menggunakan** library `HTTPClient` untuk GET request.
2. **Memparse** JSON response dengan ArduinoJson.
3. **Mengimplementasikan** retry logic dan error handling.
4. **Menampilkan** data API pada OLED.

---

## 2. Teori & Praktikum

### `platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    bblanchon/ArduinoJson@^7.0.0
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

### Praktikum — Cuaca dari Open-Meteo API + OLED

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char* SSID = "WIFI_ANDA";
const char* PASS = "PASSWORD";

// Open-Meteo API (tanpa API key!) — Jakarta
const char* WEATHER_URL =
  "https://api.open-meteo.com/v1/forecast?"
  "latitude=-6.2&longitude=106.8&current_weather=true";

float wxTemp = 0, wxWind = 0;
int wxCode = 0;
int fetchCount = 0;
bool fetchOK = false;

void fetchWeather() {
  HTTPClient http;
  http.begin(WEATHER_URL);
  http.setTimeout(10000);

  int httpCode = http.GET();
  fetchCount++;

  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      wxTemp = doc["current_weather"]["temperature"];
      wxWind = doc["current_weather"]["windspeed"];
      wxCode = doc["current_weather"]["weathercode"];
      fetchOK = true;
      Serial.printf("[#%d] Suhu: %.1f°C, Angin: %.1f km/h, Code: %d\n",
                    fetchCount, wxTemp, wxWind, wxCode);
    }
  } else {
    fetchOK = false;
    Serial.printf("[#%d] HTTP Error: %d\n", fetchCount, httpCode);
  }
  http.end();
}

const char* weatherDesc(int code) {
  if (code == 0) return "Cerah";
  if (code <= 3) return "Berawan";
  if (code <= 49) return "Berkabut";
  if (code <= 69) return "Hujan";
  if (code <= 79) return "Salju";
  if (code <= 99) return "Badai";
  return "?";
}

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Cuaca Jakarta");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  if (fetchOK) {
    display.setTextSize(2);
    display.setCursor(0, 14);
    display.printf("%.1f C", wxTemp);

    display.setTextSize(1);
    display.setCursor(0, 35);
    display.printf("Angin: %.1f km/h", wxWind);
    display.setCursor(0, 45);
    display.printf("Cuaca: %s (%d)", weatherDesc(wxCode), wxCode);
  } else {
    display.setCursor(10, 25);
    display.println("Fetching...");
  }

  display.setCursor(0, 56);
  display.printf("Fetch #%d %s", fetchCount, fetchOK?"OK":"FAIL");
  display.display();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
  }

  fetchWeather();
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 60000) { // Update tiap 1 menit
    last = millis();
    fetchWeather();
  }
  updateOLED();
  delay(500);
}
```

---

## 3. Referensi

1. **Open-Meteo.** *Weather API.* [https://open-meteo.com/](https://open-meteo.com/)
2. **Espressif.** *HTTPClient.* [https://docs.espressif.com/projects/arduino-esp32/en/latest/api/http_client.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/http_client.html)
