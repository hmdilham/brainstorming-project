# Modul Pertemuan 13 — IOT Lanjutan
# Computer Vision — OpenCV Color Tracking + MQTT Feedback

---

## 1. Maksud dan Tujuan Materi

### Maksud
Memproses video dari ESP32-CAM menggunakan **Python + OpenCV** di komputer. Komputer memiliki daya komputasi jauh lebih besar untuk CV kompleks. Pertemuan ini membahas HSV color tracking, morphological operations, contour detection, dan mengirim koordinat tracking ke ESP32 via MQTT sebagai feedback loop.

### Tujuan Pembelajaran
1. **Membaca** video stream ESP32-CAM di Python dengan OpenCV.
2. **Mendeteksi** objek berwarna menggunakan HSV color masking.
3. **Mengirim** koordinat tracking ke ESP32 via MQTT untuk kontrol servo.

---

## 2. Praktikum — Color Tracking + MQTT

### Setup Python:
```bash
pip install opencv-python numpy paho-mqtt
```

### `color_tracker.py`:
```python
"""
Color Tracking dari ESP32-CAM Stream
Mendeteksi objek merah → hitung centroid → publish ke MQTT
ESP32 subscribe → gerakkan servo ke arah objek
"""
import cv2
import numpy as np
import paho.mqtt.client as mqtt
import json
import time

# === Konfigurasi ===
ESP32_CAM_URL = "http://192.168.1.XXX:81/stream"
MQTT_BROKER = "localhost"
MQTT_TOPIC = "iot/tracking"

# HSV range untuk warna MERAH
# Merah ada di 2 range HSV (wrap-around di hue)
LOWER_RED_1 = np.array([0, 100, 100])
UPPER_RED_1 = np.array([10, 255, 255])
LOWER_RED_2 = np.array([160, 100, 100])
UPPER_RED_2 = np.array([180, 255, 255])

# MQTT
client = mqtt.Client()
client.connect(MQTT_BROKER, 1883)
client.loop_start()

# Buka stream
cap = cv2.VideoCapture(ESP32_CAM_URL)
if not cap.isOpened():
    print("Gagal buka stream!")
    exit()

print("Color Tracking aktif. Tekan ESC untuk keluar.")

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    # Konversi ke HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Mask merah (gabungan 2 range)
    mask1 = cv2.inRange(hsv, LOWER_RED_1, UPPER_RED_1)
    mask2 = cv2.inRange(hsv, LOWER_RED_2, UPPER_RED_2)
    mask = mask1 | mask2

    # Morphological: bersihkan noise
    kernel = np.ones((5, 5), np.uint8)
    mask = cv2.erode(mask, kernel, iterations=2)
    mask = cv2.dilate(mask, kernel, iterations=2)

    # Cari contour
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    if contours:
        # Ambil contour terbesar
        largest = max(contours, key=cv2.contourArea)
        area = cv2.contourArea(largest)

        if area > 500:  # Filter noise kecil
            # Bounding box dan centroid
            x, y, w, h = cv2.boundingRect(largest)
            cx = x + w // 2
            cy = y + h // 2

            # Gambar di frame
            cv2.rectangle(frame, (x, y), (x+w, y+h), (0, 255, 0), 2)
            cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)
            cv2.putText(frame, f"({cx},{cy})", (cx+10, cy-10),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

            # Publish ke MQTT
            payload = json.dumps({"x": cx, "y": cy, "w": w, "h": h, "area": int(area)})
            client.publish(MQTT_TOPIC, payload)

    # Tampilkan
    cv2.imshow("Color Tracking", frame)
    cv2.imshow("Mask", mask)

    if cv2.waitKey(1) == 27:
        break

cap.release()
cv2.destroyAllWindows()
client.loop_stop()
```

### ESP32 Subscriber: Servo Tracking

**`src/main.cpp`** (ESP32 terpisah — bukan ESP32-CAM):
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);
Servo panServo;

const char* SSID = "WIFI";
const char* PASS = "PASS";
const char* MQTT_HOST = "192.168.1.100";

void callback(char* topic, byte* payload, unsigned int len) {
  String msg = "";
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];

  JsonDocument doc;
  if (!deserializeJson(doc, msg)) {
    int cx = doc["x"];
    // Map x (0-640) ke sudut servo (0-180)
    int angle = map(constrain(cx, 0, 640), 0, 640, 0, 180);
    panServo.write(angle);
    Serial.printf("x=%d → servo=%d°\n", cx, angle);
  }
}

void setup() {
  Serial.begin(115200);
  panServo.attach(13);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  mqtt.setServer(MQTT_HOST, 1883);
  mqtt.setCallback(callback);
}

void loop() {
  if (!mqtt.connected()) {
    mqtt.connect("ESP32-Servo");
    mqtt.subscribe("iot/tracking");
  }
  mqtt.loop();
}
```

---

## 3. Referensi

1. **OpenCV.** *Documentation.* [https://docs.opencv.org/4.x/](https://docs.opencv.org/4.x/)
2. **PyImageSearch.** *Color Detection.* [https://pyimagesearch.com/](https://pyimagesearch.com/)
