# Modul Pertemuan 14 — IOT Lanjutan
# Computer Vision — YOLO Object Detection + MediaPipe Gesture Recognition

---

## 1. Maksud dan Tujuan Materi

### Maksud
Memperkenalkan dua framework CV powerful: **YOLO** (You Only Look Once) untuk deteksi objek multi-class dan **MediaPipe** untuk hand tracking dan gesture recognition. Keduanya memproses stream dari ESP32-CAM di komputer, dan hasilnya dikirim ke ESP32 via MQTT untuk kontrol aktuator.

### Tujuan Pembelajaran
1. **Menjalankan** YOLOv8n untuk deteksi objek dari stream ESP32-CAM.
2. **Menggunakan** MediaPipe Hands untuk klasifikasi gesture.
3. **Mengintegrasikan** deteksi/gesture → MQTT → aktuator ESP32.

---

## 2. Praktikum

### Setup Python:
```bash
pip install ultralytics mediapipe opencv-python paho-mqtt
```

### 2.1 — YOLO Object Detection + MQTT

```python
"""
YOLO Object Detection dari ESP32-CAM Stream
Deteksi objek → publish ke MQTT jika "person" terdeteksi
"""
from ultralytics import YOLO
import cv2
import paho.mqtt.client as mqtt
import json

model = YOLO("yolov8n.pt")  # Model nano (6MB, cepat)

ESP32_CAM = "http://192.168.1.XXX:81/stream"
client = mqtt.Client()
client.connect("localhost", 1883)

cap = cv2.VideoCapture(ESP32_CAM)

while cap.isOpened():
    ret, frame = cap.read()
    if not ret: continue

    results = model(frame, conf=0.5, verbose=False)

    persons = 0
    for r in results:
        for box in r.boxes:
            cls = int(box.cls[0])
            conf = float(box.conf[0])
            name = model.names[cls]

            if name == "person":
                persons += 1

            # Gambar bounding box
            x1, y1, x2, y2 = map(int, box.xyxy[0])
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            cv2.putText(frame, f"{name} {conf:.0%}", (x1, y1-10),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    if persons > 0:
        payload = json.dumps({"event": "person_detected", "count": persons})
        client.publish("iot/security/alert", payload)

    cv2.imshow("YOLO Detection", frame)
    if cv2.waitKey(1) == 27: break

cap.release()
cv2.destroyAllWindows()
```

### 2.2 — MediaPipe Gesture Recognition + MQTT

```python
"""
MediaPipe Hand Tracking + Gesture Classification
Gesture: OPEN (semua jari) / FIST (genggam) / POINT (telunjuk)
→ Publish gesture via MQTT → ESP32 kontrol LED/relay
"""
import cv2
import mediapipe as mp
import paho.mqtt.client as mqtt
import json

mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
mp_draw = mp.solutions.drawing_utils

client = mqtt.Client()
client.connect("localhost", 1883)

ESP32_CAM = "http://192.168.1.XXX:81/stream"
cap = cv2.VideoCapture(ESP32_CAM)

def classify_gesture(landmarks):
    """Klasifikasi gesture berdasarkan posisi jari"""
    tips = [8, 12, 16, 20]   # Index, middle, ring, pinky tip
    dips = [6, 10, 14, 18]   # Corresponding DIP joints

    fingers_up = 0
    for tip, dip in zip(tips, dips):
        if landmarks[tip].y < landmarks[dip].y:
            fingers_up += 1

    # Thumb
    if landmarks[4].x < landmarks[3].x:
        fingers_up += 1

    if fingers_up >= 4:
        return "OPEN"
    elif fingers_up <= 1 and landmarks[8].y < landmarks[6].y:
        return "POINT"
    elif fingers_up == 0:
        return "FIST"
    else:
        return f"FINGERS:{fingers_up}"

last_gesture = ""

while cap.isOpened():
    ret, frame = cap.read()
    if not ret: continue

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    result = hands.process(rgb)

    gesture = "NONE"

    if result.multi_hand_landmarks:
        for hand in result.multi_hand_landmarks:
            mp_draw.draw_landmarks(frame, hand, mp_hands.HAND_CONNECTIONS)
            gesture = classify_gesture(hand.landmark)

    # Publish hanya jika gesture berubah
    if gesture != last_gesture and gesture != "NONE":
        last_gesture = gesture
        payload = json.dumps({"gesture": gesture})
        client.publish("iot/gesture", payload)
        print(f"Gesture: {gesture}")

    cv2.putText(frame, f"Gesture: {gesture}", (10, 30),
               cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
    cv2.imshow("Gesture Control", frame)

    if cv2.waitKey(1) == 27: break

cap.release()
cv2.destroyAllWindows()
```

### 2.3 — ESP32 Aktuator: Subscribe Gesture

```cpp
// ESP32 subscribe "iot/gesture"
void callback(char* topic, byte* payload, unsigned int len) {
  JsonDocument doc;
  String msg(payload, len);
  if (!deserializeJson(doc, msg)) {
    String gesture = doc["gesture"].as<String>();
    if (gesture == "OPEN") {
      digitalWrite(PIN_LED, HIGH);      // Semua ON
      digitalWrite(PIN_RELAY, HIGH);
    } else if (gesture == "FIST") {
      digitalWrite(PIN_LED, LOW);       // Semua OFF
      digitalWrite(PIN_RELAY, LOW);
    } else if (gesture == "POINT") {
      digitalWrite(PIN_LED, HIGH);      // Hanya LED
      digitalWrite(PIN_RELAY, LOW);
    }
  }
}
```

---

## 3. Referensi

1. **Ultralytics.** *YOLOv8 Docs.* [https://docs.ultralytics.com/](https://docs.ultralytics.com/)
2. **Google.** *MediaPipe Hands.* [https://developers.google.com/mediapipe/solutions/vision/hand_landmarker](https://developers.google.com/mediapipe/solutions/vision/hand_landmarker)
