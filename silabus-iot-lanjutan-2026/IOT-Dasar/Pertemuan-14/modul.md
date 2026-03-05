# Modul Pertemuan 14 — IOT Dasar
# Web Server ESP32 & Platform IoT (Blynk & MQTT Preview)

---

## 1. Maksud dan Tujuan Materi

### Maksud
ESP32 dapat berfungsi sebagai **web server lokal** yang melayani halaman HTML — memungkinkan monitoring dan kontrol via browser tanpa cloud. Pertemuan ini juga memperkenalkan platform IoT cloud: **Blynk 2.0** untuk dashboard mobile dan **MQTT** sebagai preview protokol Pub/Sub (dibahas mendalam di IOT Lanjutan).

### Tujuan Pembelajaran
1. **Membangun** web server ESP32 dengan halaman HTML responsif.
2. **Membuat** REST API endpoint yang mengembalikan JSON.
3. **Mengimplementasikan** kontrol aktuator via tombol web.
4. **Menghubungkan** ESP32 ke Blynk 2.0 untuk monitoring mobile.
5. **Melakukan** MQTT publish/subscribe dasar sebagai preview.
6. **Menampilkan** status server pada OLED.

---

## 2. Teori Materi

### 2.1 ESP32 sebagai Web Server

```
  ┌───────────┐         WiFi (LAN)        ┌───────────────────┐
  │  Browser  │◄──── HTTP GET/POST ──────►│  ESP32 Web Server │
  │  (PC/HP)  │         Response          │  (port 80)        │
  └───────────┘       (HTML/JSON)         │  ┌─────────────┐  │
                                          │  │ Sensor DHT22│  │
                                          │  │ LED / Relay │  │
                                          │  └─────────────┘  │
                                          └───────────────────┘
```

### 2.2 ESPAsyncWebServer (Async — Non-blocking)

Library `ESPAsyncWebServer` lebih efisien dibanding `WebServer` bawaan karena bersifat asinkron — tidak memblok `loop()`.

### 2.3 MQTT Preview

MQTT (Message Queuing Telemetry Transport) adalah protokol Pub/Sub ringan:
- **Publisher** mengirim data ke **topic**
- **Subscriber** mendengarkan topic yang diminati
- **Broker** meneruskan pesan dari publisher ke subscriber

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Web Server: Dashboard Sensor + Kontrol LED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    me-no-dev/ESP Async WebServer@^1.2.4
    me-no-dev/AsyncTCP@^1.1.1
    adafruit/DHT sensor library@^1.4.6
    adafruit/Adafruit Unified Sensor@^1.1.14
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : ESP32 Web Server + REST API + OLED
 * DESKRIPSI: Web dashboard menampilkan suhu/kelembaban DHT22,
 *            tombol ON/OFF LED, endpoint JSON API.
 *            OLED menampilkan IP, jumlah request, status.
 *
 * ENDPOINT:
 *   GET /         → Halaman HTML dashboard
 *   GET /api/sensor → JSON: {"suhu":28.5,"lembap":65}
 *   GET /led/on   → Nyalakan LED
 *   GET /led/off  → Matikan LED
 *
 * RANGKAIAN:
 *   GPIO 4  → DHT22
 *   GPIO 2  → LED
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

AsyncWebServer server(80);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(4, DHT22);

const char* SSID = "NAMA_WIFI";
const char* PASS = "PASSWORD";
const uint8_t PIN_LED = 2;

bool ledState = false;
int reqCount = 0;

// HTML Page
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 IoT Dashboard</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #1a1a2e; color: #eee; text-align: center; padding: 20px; }
    .card { background: #16213e; border-radius: 15px; padding: 20px; margin: 10px auto; max-width: 350px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); }
    .value { font-size: 2.5em; font-weight: bold; color: #e94560; }
    .label { font-size: 0.9em; color: #888; margin-top: 5px; }
    .btn { display: inline-block; padding: 12px 30px; margin: 5px; border: none; border-radius: 8px; font-size: 1.1em; cursor: pointer; transition: 0.3s; }
    .btn-on { background: #00b894; color: white; }
    .btn-off { background: #e17055; color: white; }
    .btn:hover { opacity: 0.8; transform: scale(1.05); }
    h1 { color: #e94560; }
  </style>
</head>
<body>
  <h1>🌡️ ESP32 IoT Dashboard</h1>

  <div class="card">
    <div class="value" id="suhu">--</div>
    <div class="label">Suhu (°C)</div>
  </div>

  <div class="card">
    <div class="value" id="lembap">--</div>
    <div class="label">Kelembaban (%)</div>
  </div>

  <div class="card">
    <div class="label">Kontrol LED</div>
    <button class="btn btn-on" onclick="setLED('on')">💡 ON</button>
    <button class="btn btn-off" onclick="setLED('off')">OFF</button>
    <div class="label" id="ledStatus">LED: --</div>
  </div>

  <script>
    function updateData() {
      fetch('/api/sensor')
        .then(r => r.json())
        .then(d => {
          document.getElementById('suhu').innerText = d.suhu.toFixed(1);
          document.getElementById('lembap').innerText = d.lembap.toFixed(1);
          document.getElementById('ledStatus').innerText = 'LED: ' + (d.led ? 'ON' : 'OFF');
        });
    }
    function setLED(state) {
      fetch('/led/' + state).then(() => updateData());
    }
    setInterval(updateData, 2000);
    updateData();
  </script>
</body>
</html>
)rawliteral";

void updateOLED() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ESP32 Web Server");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("IP: %s", WiFi.localIP().toString().c_str());
  display.setCursor(0, 23);
  display.printf("LED: %s", ledState ? "ON" : "OFF");
  display.setCursor(0, 33);
  display.printf("Requests: %d", reqCount);
  display.setCursor(0, 43);
  display.printf("RSSI: %d dBm", WiFi.RSSI());
  display.setCursor(0, 55);
  display.printf("Up: %lu s", millis()/1000);

  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!");
  }

  // WiFi
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());

  // Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    reqCount++;
    req->send_P(200, "text/html", HTML_PAGE);
  });

  server.on("/api/sensor", HTTP_GET, [](AsyncWebServerRequest *req) {
    reqCount++;
    float s = dht.readTemperature();
    float h = dht.readHumidity();
    String json = "{\"suhu\":" + String(s,1) + ",\"lembap\":" + String(h,1) +
                  ",\"led\":" + String(ledState ? "true" : "false") + "}";
    req->send(200, "application/json", json);
  });

  server.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *req) {
    ledState = true; digitalWrite(PIN_LED, HIGH);
    req->send(200, "text/plain", "LED ON");
  });

  server.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *req) {
    ledState = false; digitalWrite(PIN_LED, LOW);
    req->send(200, "text/plain", "LED OFF");
  });

  server.begin();
  Serial.println("Web Server started on port 80");
}

void loop() {
  updateOLED();
  delay(500);
}
```

---

### Praktikum 3.2 — MQTT Preview: Publish Sensor Data

**Tambah library di `platformio.ini`:**
```ini
lib_deps =
    knolleary/PubSubClient@^2.8.0
    ; ... library lainnya
```

**`src/main.cpp` (tambahan snippet):**
```cpp
/*
 * SNIPPET: MQTT Publish Preview
 * Ditambahkan ke program utama setelah WiFi terhubung.
 * Publish data ke broker publik setiap 10 detik.
 */

#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* MQTT_BROKER = "broker.hivemq.com"; // Broker publik
const int   MQTT_PORT   = 1883;
const char* MQTT_TOPIC  = "iot/dasar/sensor";

void reconnectMQTT() {
  while (!mqtt.connected()) {
    String clientId = "ESP32-" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");
    } else {
      delay(5000);
    }
  }
}

// Dalam setup():
// mqtt.setServer(MQTT_BROKER, MQTT_PORT);

// Dalam loop():
// if (!mqtt.connected()) reconnectMQTT();
// mqtt.loop();
// static unsigned long lastPub = 0;
// if (millis() - lastPub > 10000) {
//   lastPub = millis();
//   String payload = "{\"suhu\":" + String(suhu, 1) + "}";
//   mqtt.publish(MQTT_TOPIC, payload.c_str());
//   Serial.println("MQTT Published: " + payload);
// }

// Verifikasi: buka MQTT Explorer, subscribe ke "iot/dasar/sensor"
```

---

## 4. Referensi

1. **ESPAsyncWebServer.** [https://github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
2. **Random Nerd Tutorials.** *ESP32 Web Server.* [https://randomnerdtutorials.com/esp32-web-server-arduino-ide/](https://randomnerdtutorials.com/esp32-web-server-arduino-ide/)
3. **Blynk.** *Documentation.* [https://docs.blynk.io/](https://docs.blynk.io/)
4. **PubSubClient.** [https://pubsubclient.knolleary.net/](https://pubsubclient.knolleary.net/)
