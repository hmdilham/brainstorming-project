# Sesi 16 — Color-Based Sorting (Penyortiran Warna)

## 🎯 Tujuan Pembelajaran
- Mendeteksi warna dominan objek secara real-time
- Mengklasifikasi objek ke bin berdasarkan warna
- 🔨 Mini Project: Color Sorter → ESP32 + Servo mengarahkan ke bin

---

## 📖 Teori

### Sistem Penyortiran Otomatis
```
Objek masuk → Kamera detect warna → Klasifikasi → ESP32 → Servo → Bin

  ┌─────────┐       ┌──────┐       ┌──────────┐
  │ Conveyor │──────►│Camera│──────►│  Python   │
  │  Area    │       │      │       │  HSV Mask │
  └─────────┘       └──────┘       └────┬──────┘
                                        │ Serial
                                  ┌─────▼─────┐
                                  │   ESP32    │
                                  │   Servo    │
                                  └─────┬─────┘
                              ┌─────────┼─────────┐
                              ▼         ▼         ▼
                           Bin RED   Bin GREEN  Bin BLUE
```

---

## 🛠️ Praktik

### 1. Multi-Color Detector

```python
"""
sesi16_multi_color.py
Deteksi dan klasifikasi warna dominan objek di area tertentu
"""
import cv2
import numpy as np
import imutils

# Definisi warna HSV
COLORS = {
    "RED": {
        "ranges": [
            (np.array([0, 120, 100]), np.array([10, 255, 255])),
            (np.array([170, 120, 100]), np.array([179, 255, 255]))
        ],
        "bgr": (0, 0, 255)
    },
    "GREEN": {
        "ranges": [(np.array([35, 100, 100]), np.array([85, 255, 255]))],
        "bgr": (0, 255, 0)
    },
    "BLUE": {
        "ranges": [(np.array([100, 120, 100]), np.array([130, 255, 255]))],
        "bgr": (255, 0, 0)
    },
    "YELLOW": {
        "ranges": [(np.array([20, 100, 100]), np.array([35, 255, 255]))],
        "bgr": (0, 255, 255)
    },
}


def detect_dominant_color(frame, roi_rect):
    """Deteksi warna dominan dalam ROI."""
    x, y, w, h = roi_rect
    roi = frame[y:y+h, x:x+w]
    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    hsv = cv2.GaussianBlur(hsv, (5, 5), 0)

    best_color = None
    max_pixels = 0

    for color_name, color_info in COLORS.items():
        mask = np.zeros(hsv.shape[:2], dtype=np.uint8)
        for (lower, upper) in color_info["ranges"]:
            mask = cv2.bitwise_or(mask, cv2.inRange(hsv, lower, upper))

        mask = cv2.erode(mask, None, iterations=1)
        mask = cv2.dilate(mask, None, iterations=2)

        pixel_count = cv2.countNonZero(mask)
        if pixel_count > max_pixels and pixel_count > 500:
            max_pixels = pixel_count
            best_color = color_name

    return best_color, max_pixels


cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Detection zone (area di mana objek akan diklasifikasi)
DETECT_ZONE = (220, 180, 200, 200)  # x, y, w, h

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)

    dz = DETECT_ZONE
    cv2.rectangle(frame, (dz[0], dz[1]),
                  (dz[0] + dz[2], dz[1] + dz[3]), (255, 255, 255), 2)
    cv2.putText(frame, "DETECTION ZONE", (dz[0], dz[1] - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

    # Deteksi warna
    color, pixels = detect_dominant_color(frame, DETECT_ZONE)

    if color:
        bgr = COLORS[color]["bgr"]
        cv2.rectangle(frame, (dz[0], dz[1]),
                      (dz[0] + dz[2], dz[1] + dz[3]), bgr, 3)
        cv2.putText(frame, f"{color} ({pixels}px)", (dz[0], dz[1] + dz[3] + 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, bgr, 2)

        # Bin assignment
        bin_map = {"RED": "BIN A", "GREEN": "BIN B", "BLUE": "BIN C", "YELLOW": "BIN D"}
        bin_name = bin_map.get(color, "?")
        cv2.putText(frame, f"-> {bin_name}", (dz[0], dz[1] + dz[3] + 55),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, bgr, 2)
    else:
        cv2.putText(frame, "No color detected", (dz[0], dz[1] + dz[3] + 25),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (200, 200, 200), 1)

    cv2.imshow("Color Sorter", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 2. 🔨 Mini Project: Color Sorter dengan Servo

```python
"""
sesi16_color_sorter.py
🔨 MINI PROJECT: Penyortiran warna otomatis
Kamera deteksi → Servo arahkan ke bin yang sesuai
"""
import cv2
import numpy as np
import imutils
import time

# Uncomment untuk Serial:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

COLORS = {
    "RED": {
        "ranges": [
            (np.array([0, 120, 100]), np.array([10, 255, 255])),
            (np.array([170, 120, 100]), np.array([179, 255, 255]))
        ],
        "servo_angle": 0,    # Servo position for RED bin
        "bgr": (0, 0, 255)
    },
    "GREEN": {
        "ranges": [(np.array([35, 100, 100]), np.array([85, 255, 255]))],
        "servo_angle": 90,
        "bgr": (0, 255, 0)
    },
    "BLUE": {
        "ranges": [(np.array([100, 120, 100]), np.array([130, 255, 255]))],
        "servo_angle": 180,
        "bgr": (255, 0, 0)
    },
}

cap = cv2.VideoCapture(0)
DETECT_ZONE = (220, 180, 200, 200)
last_sort = 0
SORT_COOLDOWN = 3.0  # Detik antar sort

stats = {"RED": 0, "GREEN": 0, "BLUE": 0}

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)

    dz = DETECT_ZONE
    x, y, w, h = dz
    roi = frame[y:y+h, x:x+w]
    hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
    hsv = cv2.GaussianBlur(hsv, (5, 5), 0)

    best_color = None
    max_px = 0
    for name, info in COLORS.items():
        mask = np.zeros(hsv.shape[:2], dtype=np.uint8)
        for (lo, hi) in info["ranges"]:
            mask = cv2.bitwise_or(mask, cv2.inRange(hsv, lo, hi))
        px = cv2.countNonZero(mask)
        if px > max_px and px > 800:
            max_px = px
            best_color = name

    # Auto-sort
    now = time.time()
    if best_color and now - last_sort > SORT_COOLDOWN:
        angle = COLORS[best_color]["servo_angle"]
        print(f"🔄 SORT: {best_color} → Servo {angle}°")
        # ser.write(f"SORT:{angle}\n".encode())
        stats[best_color] += 1
        last_sort = now

    # Draw
    border_color = COLORS[best_color]["bgr"] if best_color else (255, 255, 255)
    cv2.rectangle(frame, (x, y), (x+w, y+h), border_color, 2)

    if best_color:
        cv2.putText(frame, f"Color: {best_color}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, COLORS[best_color]["bgr"], 2)

    # Stats
    sy = 60
    for name, count in stats.items():
        cv2.putText(frame, f"{name}: {count}", (10, sy),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, COLORS[name]["bgr"], 2)
        sy += 25

    cv2.imshow("Color Sorter", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### ESP32 Firmware: Servo Sorter

```cpp
/**
 * sesi16_servo_sorter.ino
 * ESP32 servo controller untuk color sorting
 * Menerima "SORT:angle" via Serial
 */
#include <ESP32Servo.h>

#define SERVO_PIN 13

Servo sortServo;
int currentAngle = 90;

void setup() {
    Serial.begin(115200);
    sortServo.attach(SERVO_PIN);
    sortServo.write(90);  // Center position
    Serial.println("Color Sorter Ready!");
}

void smoothMove(int targetAngle) {
    int step = (targetAngle > currentAngle) ? 1 : -1;
    while (currentAngle != targetAngle) {
        currentAngle += step;
        sortServo.write(currentAngle);
        delay(10);
    }
}

void loop() {
    if (Serial.available()) {
        String data = Serial.readStringUntil('\n');
        data.trim();

        if (data.startsWith("SORT:")) {
            int angle = data.substring(5).toInt();
            angle = constrain(angle, 0, 180);
            Serial.printf("Sorting -> %d degrees\n", angle);
            smoothMove(angle);
            delay(500);        // Hold position
            smoothMove(90);    // Return to center
        }
    }
}
```

---

## 📝 Latihan Mandiri

1. **Tambah warna kuning dan oranye** ke sorter
2. **Buat conveyor belt simulation** — objek bergerak di frame, deteksi dan sortir
3. **Tambahkan counting** dan kirim statistik ke OLED ESP32
4. **Challenge**: Sortir berdasarkan UKURAN + WARNA (small red, big blue, dll)

---

## 📚 Referensi
- [OpenCV Color Detection](https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html)
- [ESP32 Servo Library](https://github.com/madhephaestus/ESP32Servo)
