# Modul Pertemuan 8 — IOT Lanjutan
# MQTT Part 2 — Implementasi ESP32 Publisher & Subscriber

---

## 1. Maksud dan Tujuan Materi

### Maksud
Mengintegrasikan ESP32 sebagai MQTT client yang sekaligus **Publisher** (kirim data sensor) dan **Subscriber** (terima perintah kontrol). Implementasi reconnect otomatis, non-blocking publish, dan callback.

### Tujuan Pembelajaran
1. **Mengimplementasikan** ESP32 sebagai MQTT Publisher (sensor data berkala).
2. **Mengimplementasikan** ESP32 sebagai Subscriber (terima perintah kontrol relay/LED).
3. **Menggabungkan** Pub+Sub dalam satu firmware.
4. **Menampilkan** status MQTT pada OLED.

---

## 2. Praktikum — Full-Duplex MQTT + OLED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    knolleary/PubSubClient@^2.8.0
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
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);
WiFiClient espClient;
PubSubClient mqtt(espClient);
DHT dht(4, DHT22);

const char* SSID = "WIFI_ANDA";
const char* PASS = "PASSWORD";
const char* MQTT_HOST = "broker.hivemq.com";
const int   MQTT_PORT = 1883;

const char* TOPIC_PUB = "iot/kamar/sensor";
const char* TOPIC_SUB = "iot/kamar/relay";

const uint8_t PIN_LED = 2;
bool relayState = false;
int pubCount = 0, msgCount = 0;
String lastCmd = "";

// Callback: pesan masuk dari subscription
void mqttCallback(char* topic, byte* payload, unsigned int len) {
  String msg = "";
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();
  msgCount++;
  lastCmd = msg;

  Serial.printf("[MQTT RX] %s: %s\n", topic, msg.c_str());

  // Parse command JSON
  JsonDocument doc;
  if (!deserializeJson(doc, msg)) {
    const char* relay = doc["relay"];
    if (relay) {
      relayState = (String(relay) == "ON");
      digitalWrite(PIN_LED, relayState);
    }
  } else {
    // Plain text command
    msg.toUpperCase();
    if (msg == "ON")  { relayState = true;  digitalWrite(PIN_LED, HIGH); }
    if (msg == "OFF") { relayState = false; digitalWrite(PIN_LED, LOW); }
  }
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    String clientId = "ESP32-" + String(random(0xffff), HEX);

    // LWT: pesan otomatis saat disconnect
    if (mqtt.connect(clientId.c_str(), NULL, NULL,
                     "iot/kamar/status", 1, true, "OFFLINE")) {
      Serial.println("MQTT Connected!");
      mqtt.publish("iot/kamar/status", "ONLINE", true); // Retain
      mqtt.subscribe(TOPIC_SUB, 1); // QoS 1
    } else {
      delay(5000);
    }
  }
}

void updateOLED(float suhu, float hum) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.printf("MQTT %s", mqtt.connected()?"ONLINE":"OFFLINE");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13); display.printf("T: %.1fC  H: %.1f%%", suhu, hum);
  display.setCursor(0, 23); display.printf("Relay: %s", relayState?"ON":"OFF");
  display.setCursor(0, 33); display.printf("Pub: %d  Msg: %d", pubCount, msgCount);
  display.setCursor(0, 43); display.printf("Cmd: %s", lastCmd.c_str());
  display.setCursor(0, 55); display.printf("Broker: hivemq.com");
  display.display();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.begin();

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  reconnectMQTT();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println("OLED gagal!");
}

void loop() {
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  // Publish setiap 5 detik (non-blocking)
  static unsigned long lastPub = 0;
  float suhu = dht.readTemperature();
  float hum  = dht.readHumidity();

  if (millis() - lastPub > 5000 && !isnan(suhu)) {
    lastPub = millis();
    pubCount++;

    JsonDocument doc;
    doc["suhu"] = round(suhu * 10) / 10.0;
    doc["hum"]  = round(hum * 10) / 10.0;
    doc["relay"] = relayState ? "ON" : "OFF";
    doc["up"] = millis() / 1000;

    String payload;
    serializeJson(doc, payload);
    mqtt.publish(TOPIC_PUB, payload.c_str());
    Serial.printf("[#%d] PUB: %s\n", pubCount, payload.c_str());
  }

  static unsigned long lastOLED = 0;
  if (millis() - lastOLED > 500) {
    lastOLED = millis();
    updateOLED(isnan(suhu)?0:suhu, isnan(hum)?0:hum);
  }
}
```

**Test:**
1. Buka MQTT Explorer → subscribe `iot/kamar/#`
2. Lihat data sensor masuk setiap 5 detik
3. Publish `ON` ke topic `iot/kamar/relay` → LED ESP32 menyala
4. Publish `{"relay":"OFF"}` → LED mati

---

## 3. Referensi

1. **PubSubClient.** [https://pubsubclient.knolleary.net/](https://pubsubclient.knolleary.net/)
2. **HiveMQ.** *Public Broker.* [https://www.hivemq.com/public-mqtt-broker/](https://www.hivemq.com/public-mqtt-broker/)
