# Sesi 11 — Hand Detection dengan MediaPipe

## 🎯 Tujuan Pembelajaran
- Memahami MediaPipe Hands dan 21 hand landmarks
- Mendeteksi tangan dan menggambar landmark
- Menghitung jumlah jari yang terbuka
- 🔨 Mini Project: Finger Counter → ESP32 OLED

---

## 📖 Teori

### MediaPipe Hands
MediaPipe Hands = model ML dari Google untuk tracking tangan secara real-time. Output: 21 titik landmark per tangan.

```
, , , , ,         ← Fingertips (4, 8, 12, 16, 20)
│ │ │ │ │
│ │ │ │ │         ← Per jari: TIP, DIP, PIP, MCP
│ │ │ │ │
└─┴─┴─┴─┘
    │              ← Wrist (0)
    └──
```

### 21 Landmark Points
```
           8      12     16     20
           │       │      │      │
     7     11     15     19      │
     │      │      │      │      │
     6     10     14     18      │
     │      │      │      │      │
     5      9     13     17      4
     │      │      │      │      │
     └──────┴──────┴──────┴──────3
                   │             │
                   │             2
                   │             │
                   0─────────────1
                 (WRIST)      (THUMB_CMC)
```

### Finger State Logic
Jari terbuka jika TIP lebih tinggi dari PIP (untuk 4 jari), atau jika THUMB_TIP berada di luar THUMB_IP (untuk jempol).

---

## 🛠️ Praktik

### 0. Install MediaPipe

```bash
pip install mediapipe
```

### 1. Hand Detection Dasar

```python
"""
sesi11_hand_detection.py
Deteksi tangan dan gambar landmark
"""
import cv2
import mediapipe as mp
import imutils

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils
mp_styles = mp.solutions.drawing_styles

hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=2,
    min_detection_confidence=0.7,
    min_tracking_confidence=0.5
)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)  # Mirror

    # Convert BGR → RGB untuk MediaPipe
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb)

    if results.multi_hand_landmarks:
        for hand_landmarks, handedness in zip(results.multi_hand_landmarks,
                                               results.multi_handedness):
            # Gambar landmark
            mp_draw.draw_landmarks(
                frame, hand_landmarks, mp_hands.HAND_CONNECTIONS,
                mp_styles.get_default_hand_landmarks_style(),
                mp_styles.get_default_hand_connections_style()
            )

            # Info tangan (kiri/kanan)
            hand_label = handedness.classification[0].label
            conf = handedness.classification[0].score

            # Posisi wrist untuk label
            wrist = hand_landmarks.landmark[0]
            h, w, _ = frame.shape
            wx, wy = int(wrist.x * w), int(wrist.y * h)

            cv2.putText(frame, f"{hand_label} ({conf:.0%})", (wx, wy - 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    num_hands = len(results.multi_hand_landmarks) if results.multi_hand_landmarks else 0
    cv2.putText(frame, f"Hands: {num_hands}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    cv2.imshow("Hand Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### 2. Hand Landmark Coordinates

```python
"""
sesi11_landmark_coords.py
Menampilkan koordinat setiap landmark
"""
import cv2
import mediapipe as mp
import imutils

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

# Nama landmark
LANDMARK_NAMES = {
    0: "WRIST", 1: "THUMB_CMC", 2: "THUMB_MCP", 3: "THUMB_IP", 4: "THUMB_TIP",
    5: "INDEX_MCP", 6: "INDEX_PIP", 7: "INDEX_DIP", 8: "INDEX_TIP",
    9: "MIDDLE_MCP", 10: "MIDDLE_PIP", 11: "MIDDLE_DIP", 12: "MIDDLE_TIP",
    13: "RING_MCP", 14: "RING_PIP", 15: "RING_DIP", 16: "RING_TIP",
    17: "PINKY_MCP", 18: "PINKY_PIP", 19: "PINKY_DIP", 20: "PINKY_TIP"
}

hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = hands.process(rgb)

    if results.multi_hand_landmarks:
        for hand_lm in results.multi_hand_landmarks:
            mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

            # Tampilkan ID setiap landmark
            for idx, lm in enumerate(hand_lm.landmark):
                px, py = int(lm.x * w), int(lm.y * h)
                cv2.circle(frame, (px, py), 3, (0, 0, 255), -1)
                cv2.putText(frame, str(idx), (px + 5, py - 5),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.3, (255, 0, 0), 1)

    cv2.imshow("Landmarks", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### 3. Finger Counter

```python
"""
sesi11_finger_counter.py
Menghitung jumlah jari yang terbuka
"""
import cv2
import mediapipe as mp
import imutils

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

# Landmark tips dan PIP untuk setiap jari
FINGER_TIPS = [8, 12, 16, 20]   # Index, Middle, Ring, Pinky
FINGER_PIPS = [6, 10, 14, 18]   # PIP joint masing-masing
THUMB_TIP = 4
THUMB_IP = 3
THUMB_MCP = 2


def count_fingers(hand_landmarks, handedness_label):
    """Hitung jumlah jari terbuka."""
    landmarks = hand_landmarks.landmark
    fingers = []

    # Thumb — based on x position (horizontal)
    if handedness_label == "Right":
        # Tangan kanan (di mirror): thumb tip lebih kiri dari IP
        if landmarks[THUMB_TIP].x < landmarks[THUMB_IP].x:
            fingers.append(1)
        else:
            fingers.append(0)
    else:
        if landmarks[THUMB_TIP].x > landmarks[THUMB_IP].x:
            fingers.append(1)
        else:
            fingers.append(0)

    # 4 jari lainnya — based on y position (vertical)
    for tip, pip in zip(FINGER_TIPS, FINGER_PIPS):
        if landmarks[tip].y < landmarks[pip].y:  # Tip lebih atas = terbuka
            fingers.append(1)
        else:
            fingers.append(0)

    return fingers


hands = mp_hands.Hands(max_num_hands=2, min_detection_confidence=0.7)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = hands.process(rgb)

    total_fingers = 0

    if results.multi_hand_landmarks:
        for hand_lm, handedness in zip(results.multi_hand_landmarks,
                                        results.multi_handedness):
            mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

            label = handedness.classification[0].label
            fingers = count_fingers(hand_lm, label)
            finger_count = sum(fingers)
            total_fingers += finger_count

            # Tampilkan per tangan
            h, w, _ = frame.shape
            wrist = hand_lm.landmark[0]
            wx, wy = int(wrist.x * w), int(wrist.y * h)

            finger_names = ["👍", "☝️", "✌️", "🤟", "🖐"]
            status = finger_names[min(finger_count, 4)] if finger_count > 0 else "✊"

            cv2.putText(frame, f"{label}: {finger_count} {status}",
                        (wx - 40, wy + 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # Total count besar di pojok
    cv2.putText(frame, str(total_fingers), (540, 70),
                cv2.FONT_HERSHEY_SIMPLEX, 2.5, (0, 255, 0), 4)
    cv2.putText(frame, "fingers", (540, 95),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 1)

    cv2.imshow("Finger Counter", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### 4. 🔨 Mini Project: Finger Counter → ESP32 OLED

```python
"""
sesi11_finger_to_esp32.py
🔨 MINI PROJECT: Jumlah jari → kirim ke ESP32 → tampilkan di OLED
"""
import cv2
import mediapipe as mp
import imutils
import time

# Uncomment untuk Serial:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

FINGER_TIPS = [8, 12, 16, 20]
FINGER_PIPS = [6, 10, 14, 18]
THUMB_TIP, THUMB_IP = 4, 3


def count_fingers(hand_landmarks, hand_label):
    lm = hand_landmarks.landmark
    fingers = []

    if hand_label == "Right":
        fingers.append(1 if lm[THUMB_TIP].x < lm[THUMB_IP].x else 0)
    else:
        fingers.append(1 if lm[THUMB_TIP].x > lm[THUMB_IP].x else 0)

    for tip, pip in zip(FINGER_TIPS, FINGER_PIPS):
        fingers.append(1 if lm[tip].y < lm[pip].y else 0)

    return sum(fingers)


hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)

last_count = -1
last_send_time = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb)

    finger_count = 0
    if results.multi_hand_landmarks:
        hand_lm = results.multi_hand_landmarks[0]
        label = results.multi_handedness[0].classification[0].label
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)
        finger_count = count_fingers(hand_lm, label)

    # Kirim ke ESP32 jika berubah
    now = time.time()
    if finger_count != last_count and now - last_send_time > 0.3:
        last_count = finger_count
        last_send_time = now
        cmd = f"FINGERS:{finger_count}"
        print(f"  >> {cmd}")
        # ser.write(f"{cmd}\n".encode())

    cv2.putText(frame, str(finger_count), (540, 70),
                cv2.FONT_HERSHEY_SIMPLEX, 2.5, (0, 255, 0), 4)
    cv2.imshow("Finger → ESP32", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Test** hand detection dengan kedua tangan secara bersamaan
2. **Coba** dari berbagai sudut — kapan deteksi mulai gagal?
3. **Modifikasi** finger counter agar bisa menampilkan nama gesture (✊, ☝️, ✌️, 🖐)
4. **Kirim** jumlah jari ke ESP32 dan tampilkan angka besar di OLED

---

## 📚 Referensi
- [MediaPipe Hands](https://google.github.io/mediapipe/solutions/hands.html)
- [Hand Landmark Model](https://ai.google.dev/edge/mediapipe/solutions/vision/hand_landmarker)
