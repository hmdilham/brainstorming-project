# Sesi 14 — Virtual Touch & Tombol Virtual

## 🎯 Tujuan Pembelajaran
- Membuat tombol virtual di overlay video
- Mendeteksi "sentuhan" jari pada zona tombol
- 🔨 Mini Project: Virtual Button Panel untuk kontrol perangkat ESP32

---

## 📖 Teori

### Virtual Touch Concept
Kita mendefinisikan area persegi di layar video sebagai "tombol". Jika ujung jari telunjuk (landmark 8) berada di dalam area tersebut DAN jari sedang dalam posisi "menekan" (jarak index tip ke index MCP pendek), maka tombol dianggap ditekan.

```
  ┌──────────────────────────────────────┐
  │                                      │
  │   ┌──────┐  ┌──────┐  ┌──────┐     │
  │   │ LED1 │  │ LED2 │  │ FAN  │     │  ← Virtual buttons
  │   │  ON  │  │  ON  │  │  ON  │     │
  │   └──────┘  └──────┘  └──────┘     │
  │                                      │
  │           ☝️ ← fingertip             │
  │                                      │
  └──────────────────────────────────────┘
```

---

## 🛠️ Praktik

### 1. Virtual Button System

```python
"""
sesi14_virtual_buttons.py
Sistem tombol virtual yang bisa ditekan dengan jari
"""
import cv2
import mediapipe as mp
import imutils
import numpy as np
import time

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

# ====== Definisi Tombol Virtual ======
class VirtualButton:
    def __init__(self, x, y, w, h, label, color=(100, 100, 100)):
        self.x = x
        self.y = y
        self.w = w
        self.h = h
        self.label = label
        self.color = color
        self.state = False
        self.pressed = False
        self.last_toggle_time = 0

    def contains(self, px, py):
        return (self.x <= px <= self.x + self.w and
                self.y <= py <= self.y + self.h)

    def toggle(self):
        now = time.time()
        if now - self.last_toggle_time > 0.5:  # Debounce 500ms
            self.state = not self.state
            self.last_toggle_time = now
            return True
        return False

    def draw(self, frame):
        if self.state:
            bg_color = (0, 200, 0)  # Hijau = ON
        elif self.pressed:
            bg_color = (0, 150, 200)  # Highlight saat hover
        else:
            bg_color = self.color

        # Background
        cv2.rectangle(frame, (self.x, self.y),
                      (self.x + self.w, self.y + self.h), bg_color, -1)
        # Border
        cv2.rectangle(frame, (self.x, self.y),
                      (self.x + self.w, self.y + self.h), (255, 255, 255), 2)

        # Label
        status = "ON" if self.state else "OFF"
        cv2.putText(frame, self.label, (self.x + 10, self.y + 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
        cv2.putText(frame, status, (self.x + 10, self.y + 50),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6,
                    (0, 255, 0) if self.state else (100, 100, 100), 2)


# Buat tombol-tombol
buttons = [
    VirtualButton(50,  50, 120, 70, "LED 1"),
    VirtualButton(200, 50, 120, 70, "LED 2"),
    VirtualButton(350, 50, 120, 70, "FAN"),
    VirtualButton(500, 50, 120, 70, "BUZZER"),
]

hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)

# Uncomment untuk Serial:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

def is_finger_pressing(hand_lm):
    """Check if index finger is in pressing position."""
    lm = hand_lm.landmark
    # Jarak antara index tip dan index MCP
    tip_y = lm[8].y
    mcp_y = lm[5].y
    pip_y = lm[6].y

    # Jari menunjuk ke bawah (pressing) jika tip dibawah PIP
    # atau jika jari lurus ke depan
    return tip_y < pip_y  # Jari masih terbuka/menunjuk

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape

    # Buat overlay semi-transparan
    overlay = frame.copy()

    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb)

    finger_pos = None

    if results.multi_hand_landmarks:
        hand_lm = results.multi_hand_landmarks[0]
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

        # Posisi ujung jari telunjuk (landmark 8)
        idx_tip = hand_lm.landmark[8]
        fx, fy = int(idx_tip.x * w), int(idx_tip.y * h)
        finger_pos = (fx, fy)

        # Gambar pointer
        cv2.circle(frame, (fx, fy), 8, (0, 0, 255), -1)
        cv2.circle(frame, (fx, fy), 12, (0, 0, 255), 2)

        # Cek apakah jari terbuka
        finger_active = is_finger_pressing(hand_lm)

        for btn in buttons:
            btn.pressed = btn.contains(fx, fy)

            if btn.pressed and finger_active:
                if btn.toggle():
                    action = f"{btn.label}_{'ON' if btn.state else 'OFF'}"
                    print(f"⚡ {action}")
                    # ser.write(f"{action}\n".encode())
    else:
        for btn in buttons:
            btn.pressed = False

    # Gambar tombol
    for btn in buttons:
        btn.draw(frame)

    # Status bar
    cv2.putText(frame, "Virtual Touch Panel", (10, h - 20),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (200, 200, 200), 1)

    if finger_pos:
        cv2.putText(frame, f"Finger: ({finger_pos[0]}, {finger_pos[1]})",
                    (10, h - 40), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                    (200, 200, 200), 1)

    cv2.imshow("Virtual Buttons", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

### 2. 🔨 Mini Project: Virtual Slider Control

```python
"""
sesi14_virtual_slider.py
🔨 MINI PROJECT: Slider virtual untuk kontrol kecerahan LED (PWM)
"""
import cv2
import mediapipe as mp
import imutils
import numpy as np
import time

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

class VirtualSlider:
    def __init__(self, x, y, w, h, label, min_val=0, max_val=255):
        self.x = x
        self.y = y
        self.w = w
        self.h = h
        self.label = label
        self.min_val = min_val
        self.max_val = max_val
        self.value = min_val
        self.active = False

    def update(self, finger_x, finger_y):
        if (self.x <= finger_x <= self.x + self.w and
            self.y - 20 <= finger_y <= self.y + self.h + 20):
            self.active = True
            ratio = (finger_x - self.x) / self.w
            ratio = max(0, min(1, ratio))
            self.value = int(self.min_val + ratio * (self.max_val - self.min_val))
            return True
        self.active = False
        return False

    def draw(self, frame):
        # Background track
        cv2.rectangle(frame, (self.x, self.y),
                      (self.x + self.w, self.y + self.h), (80, 80, 80), -1)

        # Fill
        fill_w = int((self.value - self.min_val) / (self.max_val - self.min_val) * self.w)
        color = (0, int(self.value), 0) if not self.active else (0, 200, 200)
        cv2.rectangle(frame, (self.x, self.y),
                      (self.x + fill_w, self.y + self.h), color, -1)

        # Border
        border_color = (0, 255, 255) if self.active else (200, 200, 200)
        cv2.rectangle(frame, (self.x, self.y),
                      (self.x + self.w, self.y + self.h), border_color, 2)

        # Knob
        knob_x = self.x + fill_w
        knob_y = self.y + self.h // 2
        cv2.circle(frame, (knob_x, knob_y), 10, (255, 255, 255), -1)

        # Label dan value
        cv2.putText(frame, f"{self.label}: {self.value}",
                    (self.x, self.y - 5),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)


# Buat slider
sliders = [
    VirtualSlider(50, 100, 400, 20, "LED Brightness"),
    VirtualSlider(50, 160, 400, 20, "Fan Speed"),
    VirtualSlider(50, 220, 400, 20, "Servo Angle", 0, 180),
]

hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)
cap = cv2.VideoCapture(0)

last_send = {}

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
        hand_lm = results.multi_hand_landmarks[0]
        mp_draw.draw_landmarks(frame, hand_lm, mp_hands.HAND_CONNECTIONS)

        # Index fingertip
        tip = hand_lm.landmark[8]
        fx, fy = int(tip.x * w), int(tip.y * h)
        cv2.circle(frame, (fx, fy), 6, (0, 0, 255), -1)

        for slider in sliders:
            if slider.update(fx, fy):
                now = time.time()
                if slider.label not in last_send or now - last_send[slider.label] > 0.2:
                    last_send[slider.label] = now
                    print(f"  >> {slider.label} = {slider.value}")
                    # ser.write(f"SET:{slider.label}:{slider.value}\n".encode())

    for slider in sliders:
        slider.draw(frame)

    cv2.putText(frame, "Virtual Sliders — control with finger", (10, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    cv2.imshow("Virtual Sliders", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

hands.close()
cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Tambahkan tombol** untuk semua GPIO yang tersedia di ESP32 DevKit
2. **Buat color picker virtual**: grid warna yang bisa dipilih dengan jari
3. **Gabungkan** tombol dan slider dalam satu panel kontrol
4. **Challenge**: Buat virtual keyboard QWERTY sederhana

---

## 📚 Referensi
- [MediaPipe Virtual Mouse](https://google.github.io/mediapipe/solutions/hands.html)
- [OpenCV GUI](https://docs.opencv.org/4.x/d6/d00/tutorial_py_root.html)
