# Sesi 13 — Gesture untuk Kontrol Arah (Steering RC Car)

## 🎯 Tujuan Pembelajaran
- Mendeteksi arah tangan (kiri/kanan/atas/bawah)
- Mapping gesture arah ke perintah steering
- 🔨 Mini Project: Kontrol mobil RC dengan gesture tangan

---

## 📖 Teori

### Directional Gesture Detection
Kita menggunakan posisi relatif landmark tangan untuk menentukan arah:
- **Kiri/Kanan**: posisi horizontal centroid tangan relatif ke tengah frame
- **Maju/Mundur**: posisi vertikal tangan (atas=maju, bawah=mundur)
- **Tilt**: sudut garis antara wrist dan middle finger MCP

```
        ┌─────────────┐
        │  MAJU/STOP  │
        │             │
  KIRI  │   CENTER    │  KANAN
        │             │
        │   MUNDUR    │
        └─────────────┘
```

---

## 🛠️ Praktik

### 1. Directional Hand Tracking

```python
"""
sesi13_direction_detect.py
Deteksi arah tangan: kiri, kanan, atas, bawah
"""
import cv2
import mediapipe as mp
import imutils
import numpy as np
import math

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Zone boundaries (fraction of frame width/height)
LEFT_ZONE = 0.35
RIGHT_ZONE = 0.65
TOP_ZONE = 0.35
BOTTOM_ZONE = 0.65

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = hands.process(rgb)

    direction_h = "CENTER"
    direction_v = "CENTER"

    # Gambar zona
    cv2.line(frame, (int(w * LEFT_ZONE), 0), (int(w * LEFT_ZONE), h), (100, 100, 100), 1)
    cv2.line(frame, (int(w * RIGHT_ZONE), 0), (int(w * RIGHT_ZONE), h), (100, 100, 100), 1)
    cv2.line(frame, (0, int(h * TOP_ZONE)), (w, int(h * TOP_ZONE)), (100, 100, 100), 1)
    cv2.line(frame, (0, int(h * BOTTOM_ZONE)), (w, int(h * BOTTOM_ZONE)), (100, 100, 100), 1)

    if results.multi_hand_landmarks:
        hand_lm = results.multi_hand_landmarks[0]
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

        # Hitung centroid tangan (rata-rata semua landmark)
        cx = sum([lm.x for lm in hand_lm.landmark]) / 21
        cy = sum([lm.y for lm in hand_lm.landmark]) / 21

        # Pixel coordinates
        px, py = int(cx * w), int(cy * h)
        cv2.circle(frame, (px, py), 10, (0, 0, 255), -1)

        # Determine direction
        if cx < LEFT_ZONE:
            direction_h = "LEFT"
        elif cx > RIGHT_ZONE:
            direction_h = "RIGHT"

        if cy < TOP_ZONE:
            direction_v = "FORWARD"
        elif cy > BOTTOM_ZONE:
            direction_v = "BACKWARD"

        # Hitung tilt angle (sudut garis wrist → middle MCP)
        wrist = hand_lm.landmark[0]
        mid_mcp = hand_lm.landmark[9]
        angle = math.degrees(math.atan2(mid_mcp.y - wrist.y,
                                         mid_mcp.x - wrist.x))
        cv2.putText(frame, f"Tilt: {angle:.0f}°", (10, 90),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)

    # Status
    cv2.putText(frame, f"H: {direction_h}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
    cv2.putText(frame, f"V: {direction_v}", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    cv2.imshow("Direction Detect", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### 2. 🔨 Mini Project: RC Car Steering via Gesture

```python
"""
sesi13_rc_car_gesture.py
🔨 MINI PROJECT: Kontrol mobil RC dengan gesture tangan

Gesture mapping:
  Tangan di KIRI    → belok kiri
  Tangan di KANAN   → belok kanan
  Tangan di ATAS    → maju
  Tangan di BAWAH   → mundur
  Tidak ada tangan  → stop
  ✊ Fist           → stop
"""
import cv2
import mediapipe as mp
import imutils
import time

# Uncomment untuk Serial ESP32:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

FINGER_TIPS = [8, 12, 16, 20]
FINGER_PIPS = [6, 10, 14, 18]


def is_open_hand(hand_lm, hand_label):
    """Check if hand is open (at least 3 fingers up)."""
    lm = hand_lm.landmark
    count = 0
    for tip, pip in zip(FINGER_TIPS, FINGER_PIPS):
        if lm[tip].y < lm[pip].y:
            count += 1
    return count >= 3


hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)

last_command = ""
send_interval = 0.1  # 100ms
last_send_time = 0

# Zone thresholds
DEAD_ZONE = 0.15  # 15% dead zone di tengah

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = hands.process(rgb)

    command = "STOP"
    steer = "STRAIGHT"
    speed_text = "0"

    if results.multi_hand_landmarks:
        hand_lm = results.multi_hand_landmarks[0]
        label = results.multi_handedness[0].classification[0].label
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

        if is_open_hand(hand_lm, label):
            # Centroid tangan
            cx = sum([lm.x for lm in hand_lm.landmark]) / 21
            cy = sum([lm.y for lm in hand_lm.landmark]) / 21

            # Horizontal: steering
            offset_x = cx - 0.5  # -0.5 (kiri) → +0.5 (kanan)
            if abs(offset_x) > DEAD_ZONE:
                steer = "LEFT" if offset_x < 0 else "RIGHT"
                steer_val = int(abs(offset_x) * 200)  # 0-100 steering intensity
            else:
                steer_val = 0

            # Vertical: speed
            offset_y = cy - 0.5  # -0.5 (atas/maju) → +0.5 (bawah/mundur)
            if offset_y < -DEAD_ZONE:
                command = "FORWARD"
                speed_val = int(abs(offset_y) * 200)
            elif offset_y > DEAD_ZONE:
                command = "BACKWARD"
                speed_val = int(abs(offset_y) * 200)
            else:
                command = "STOP"
                speed_val = 0

            speed_text = str(min(speed_val, 100))

            # Visual: arrow indicator
            px, py = int(cx * w), int(cy * h)
            cv2.circle(frame, (px, py), 8, (0, 0, 255), -1)
        else:
            # Fist = brake
            command = "STOP"
            steer = "STRAIGHT"
    else:
        command = "STOP"
        steer = "STRAIGHT"

    # Kirim ke ESP32
    now = time.time()
    full_command = f"{command},{steer}"
    if full_command != last_command and now - last_send_time > send_interval:
        last_command = full_command
        last_send_time = now
        print(f"  >> {full_command}")
        # ser.write(f"{full_command}\n".encode())

    # Dashboard visual
    dash_y = h - 80

    # Command (maju/mundur/stop)
    cmd_color = {"FORWARD": (0, 255, 0), "BACKWARD": (0, 128, 255),
                 "STOP": (0, 0, 255)}.get(command, (200, 200, 200))
    cv2.putText(frame, f"Speed: {command} ({speed_text}%)", (10, dash_y),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, cmd_color, 2)

    # Steering
    steer_color = {"LEFT": (255, 0, 0), "RIGHT": (0, 255, 0),
                   "STRAIGHT": (200, 200, 200)}.get(steer, (200, 200, 200))
    cv2.putText(frame, f"Steer: {steer}", (10, dash_y + 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, steer_color, 2)

    # Mini car visualization
    car_x = w // 2
    car_y = dash_y - 20
    cv2.rectangle(frame, (car_x - 15, car_y - 20), (car_x + 15, car_y + 20),
                  (255, 255, 255), 2)
    if command == "FORWARD":
        cv2.arrowedLine(frame, (car_x, car_y + 10), (car_x, car_y - 30),
                        (0, 255, 0), 2)
    elif command == "BACKWARD":
        cv2.arrowedLine(frame, (car_x, car_y - 10), (car_x, car_y + 30),
                        (0, 128, 255), 2)
    if steer == "LEFT":
        cv2.arrowedLine(frame, (car_x + 10, car_y), (car_x - 30, car_y),
                        (255, 0, 0), 2)
    elif steer == "RIGHT":
        cv2.arrowedLine(frame, (car_x - 10, car_y), (car_x + 30, car_y),
                        (0, 255, 0), 2)

    cv2.imshow("RC Car Gesture Control", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### ESP32 Firmware: Motor Driver L298N

```cpp
/**
 * sesi13_rc_car.ino
 * ESP32 mengontrol motor DC via L298N berdasarkan perintah Serial
 * Format: "COMMAND,STEER"  (e.g., "FORWARD,LEFT")
 */
#define ENA  14   // PWM kecepatan motor kiri
#define IN1  27   // Motor kiri
#define IN2  26
#define ENB  25   // PWM kecepatan motor kanan
#define IN3  33   // Motor kanan
#define IN4  32

const int SPEED = 200;  // 0-255
const int TURN_SPEED = 150;

void setup() {
    Serial.begin(115200);
    pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
    stopMotors();
    Serial.println("RC Car Ready!");
}

void stopMotors() {
    analogWrite(ENA, 0); analogWrite(ENB, 0);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void forward() {
    analogWrite(ENA, SPEED); analogWrite(ENB, SPEED);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void backward() {
    analogWrite(ENA, SPEED); analogWrite(ENB, SPEED);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnLeft() {
    analogWrite(ENA, TURN_SPEED); analogWrite(ENB, SPEED);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnRight() {
    analogWrite(ENA, SPEED); analogWrite(ENB, TURN_SPEED);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        data.trim();

        int commaIdx = data.indexOf(',');
        String cmd = data.substring(0, commaIdx);
        String steer = data.substring(commaIdx + 1);

        if (cmd == "STOP") {
            stopMotors();
        } else if (cmd == "FORWARD") {
            if (steer == "LEFT") turnLeft();
            else if (steer == "RIGHT") turnRight();
            else forward();
        } else if (cmd == "BACKWARD") {
            backward();
        }

        Serial.printf("CMD: %s, STEER: %s\n", cmd.c_str(), steer.c_str());
    }
}
```

---

## 📝 Latihan Mandiri

1. **Tambahkan speed control**: jarak tangan dari center = kecepatan (PWM)
2. **Buat mode "tank steering"**: dua tangan, masing-masing kontrol satu roda
3. **Tambahkan gesture khusus** untuk klakson (misal ✌️ = honk)
4. **Challenge**: Kontrol servo tambahan untuk kamera pan/tilt di mobil RC

---

## 📚 Referensi
- [L298N Motor Driver](https://lastminuteengineers.com/l298n-dc-stepper-driver-arduino-tutorial/)
- [ESP32 PWM](https://randomnerdtutorials.com/esp32-pwm-arduino-ide/)
