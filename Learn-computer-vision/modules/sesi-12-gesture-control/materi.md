# Sesi 12 — Gesture Recognition untuk Kontrol Perangkat

## 🎯 Tujuan Pembelajaran
- Mendefinisikan dan mengklasifikasi gesture tangan
- Mapping gesture ke aksi perangkat
- 🔨 Mini Project: ON/OFF Lampu dengan gesture telapak tangan

---

## 📖 Teori

### Gesture Classification dari Landmark
Dengan mengetahui posisi 21 landmark, kita bisa mengklasifikasi gesture berdasarkan:
- **Jari mana yang terbuka/tertutup**
- **Posisi relatif antar landmark**
- **Sudut antar segmen jari**

### Gesture yang Akan Dikenali
| Gesture | Deskripsi | Aksi |
|---------|-----------|------|
| ✋ Open Palm | Semua jari terbuka | LED ON / Relay ON |
| ✊ Fist | Semua jari tertutup | LED OFF / Relay OFF |
| ☝️ Point Up | Hanya telunjuk terbuka | Aksi 1 |
| ✌️ Peace | Telunjuk + tengah terbuka | Aksi 2 |
| 👍 Thumbs Up | Hanya jempol terbuka | OK / Confirm |
| 👎 Thumbs Down | Jempol ke bawah | Cancel |

---

## 🛠️ Praktik

### 1. Gesture Classifier

```python
"""
sesi12_gesture_classifier.py
Klasifikasi gesture tangan berdasarkan posisi landmark
"""
import cv2
import mediapipe as mp
import imutils

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

FINGER_TIPS = [8, 12, 16, 20]
FINGER_PIPS = [6, 10, 14, 18]
THUMB_TIP, THUMB_IP = 4, 3


def get_finger_states(hand_landmarks, hand_label):
    """Return list [thumb, index, middle, ring, pinky] — 1=open, 0=closed."""
    lm = hand_landmarks.landmark
    states = []

    # Thumb
    if hand_label == "Right":
        states.append(1 if lm[THUMB_TIP].x < lm[THUMB_IP].x else 0)
    else:
        states.append(1 if lm[THUMB_TIP].x > lm[THUMB_IP].x else 0)

    # Fingers
    for tip, pip in zip(FINGER_TIPS, FINGER_PIPS):
        states.append(1 if lm[tip].y < lm[pip].y else 0)

    return states


def classify_gesture(finger_states):
    """Klasifikasi gesture berdasarkan state jari."""
    thumb, index, middle, ring, pinky = finger_states

    # All open = Open Palm
    if all(finger_states):
        return "OPEN_PALM", "✋"

    # All closed = Fist
    if not any(finger_states):
        return "FIST", "✊"

    # Only index = Point
    if finger_states == [0, 1, 0, 0, 0]:
        return "POINT", "☝️"

    # Index + Middle = Peace
    if finger_states == [0, 1, 1, 0, 0]:
        return "PEACE", "✌️"

    # Only thumb = Thumbs Up
    if finger_states == [1, 0, 0, 0, 0]:
        return "THUMBS_UP", "👍"

    # Index + Pinky = Rock
    if finger_states == [0, 1, 0, 0, 1]:
        return "ROCK", "🤘"

    # Thumb + Pinky = Call
    if finger_states == [1, 0, 0, 0, 1]:
        return "CALL", "🤙"

    # Three fingers
    if sum(finger_states) == 3:
        return "THREE", "3️⃣"

    # Four fingers (no thumb)
    if finger_states == [0, 1, 1, 1, 1]:
        return "FOUR", "4️⃣"

    return "UNKNOWN", "❓"


hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
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

    gesture_name = "NO HAND"
    gesture_emoji = ""

    if results.multi_hand_landmarks:
        hand_lm = results.multi_hand_landmarks[0]
        label = results.multi_handedness[0].classification[0].label
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

        states = get_finger_states(hand_lm, label)
        gesture_name, gesture_emoji = classify_gesture(states)

        # Tampilkan finger states
        finger_names = ["T", "I", "M", "R", "P"]
        state_str = " ".join([f"{n}:{'O' if s else 'X'}"
                              for n, s in zip(finger_names, states)])
        cv2.putText(frame, state_str, (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)

    cv2.putText(frame, f"Gesture: {gesture_name} {gesture_emoji}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    cv2.imshow("Gesture Recognition", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### 2. 🔨 Mini Project: ON/OFF Lampu dengan Gesture

```python
"""
sesi12_gesture_lamp.py
🔨 MINI PROJECT: ON/OFF Lampu dengan gesture

- ✋ Open Palm  = LAMPU ON  (Relay ON)
- ✊ Fist       = LAMPU OFF (Relay OFF)
- ☝️ Point     = Toggle mode
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


def get_finger_states(hand_lm, hand_label):
    lm = hand_lm.landmark
    states = []
    if hand_label == "Right":
        states.append(1 if lm[4].x < lm[3].x else 0)
    else:
        states.append(1 if lm[4].x > lm[3].x else 0)
    for tip, pip in zip(FINGER_TIPS, FINGER_PIPS):
        states.append(1 if lm[tip].y < lm[pip].y else 0)
    return states


def classify_gesture(states):
    if all(states):
        return "OPEN_PALM"
    if not any(states):
        return "FIST"
    if states == [0, 1, 0, 0, 0]:
        return "POINT"
    if states == [1, 0, 0, 0, 0]:
        return "THUMBS_UP"
    return "OTHER"


hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)

# State
lamp_state = False
last_gesture = None
gesture_hold_time = 0
HOLD_REQUIRED = 0.5  # Harus tahan gesture 0.5 detik

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb)

    gesture = "NONE"
    if results.multi_hand_landmarks:
        hand_lm = results.multi_hand_landmarks[0]
        label = results.multi_handedness[0].classification[0].label
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)
        states = get_finger_states(hand_lm, label)
        gesture = classify_gesture(states)

    # Debounce: gesture harus ditahan beberapa saat
    now = time.time()
    if gesture == last_gesture:
        if now - gesture_hold_time >= HOLD_REQUIRED:
            if gesture == "OPEN_PALM" and not lamp_state:
                lamp_state = True
                print("💡 LAMP ON!")
                # ser.write(b"RELAY_ON\n")
            elif gesture == "FIST" and lamp_state:
                lamp_state = False
                print("🌑 LAMP OFF!")
                # ser.write(b"RELAY_OFF\n")
    else:
        last_gesture = gesture
        gesture_hold_time = now

    # Visual feedback
    # Simulasi lampu di layar
    lamp_color = (0, 255, 255) if lamp_state else (50, 50, 50)
    cv2.circle(frame, (580, 50), 30, lamp_color, -1)
    cv2.circle(frame, (580, 50), 32, (200, 200, 200), 2)

    status = "ON" if lamp_state else "OFF"
    cv2.putText(frame, f"LAMP: {status}", (520, 95),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, lamp_color, 2)

    cv2.putText(frame, f"Gesture: {gesture}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    # Petunjuk
    cv2.putText(frame, "Open Palm = ON | Fist = OFF", (10, frame.shape[0] - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

    cv2.imshow("Gesture Lamp Control", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Tambahkan gesture baru**: ✌️ Peace = toggle LED kedua
2. **Implementasi brightness control**: jumlah jari = tingkat kecerahan LED (PWM)
3. **Buat multi-device control**: gesture berbeda untuk perangkat berbeda (lampu, kipas, dll)
4. **Challenge**: Buat gesture "sequence" — misal ✌️ lalu ✊ = unlock

---

## 📚 Referensi
- [MediaPipe Hand Gesture Recognition](https://google.github.io/mediapipe/solutions/hands.html)
- [Gesture Control Tutorial](https://techvidvan.com/tutorials/hand-gesture-recognition-tensorflow-opencv/)
