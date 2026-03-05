# Sesi 9 — Object Detection dengan YOLO

## 🎯 Tujuan Pembelajaran
- Memahami konsep YOLO (You Only Look Once) dan arsitekturnya
- Setup dan jalankan YOLOv8 untuk object detection
- Deteksi objek dari webcam / ESP32-CAM stream
- 🔨 Mini Project: People Counter → tampilkan di OLED ESP32

---

## 📖 Teori

### Apa itu YOLO?
YOLO = **Y**ou **O**nly **L**ook **O**nce — algoritma object detection yang memprediksi bounding box dan kelas objek dalam satu forward pass.

```
Input Image ──► YOLO Model ──► Bounding Boxes + Class + Confidence
                                 ┌──────────────────────┐
                                 │ person (0.95)         │
                                 │ car (0.87)            │
                                 │ bottle (0.72)         │
                                 └──────────────────────┘
```

### Model Size Comparison (YOLOv8)
| Model | Size | Speed (ms) | mAP | Use Case |
|-------|:----:|:----------:|:---:|----------|
| YOLOv8n (nano) | 6MB | ~5ms | 37.3 | Edge/ESP32 |
| YOLOv8s (small) | 22MB | ~10ms | 44.9 | Balanced |
| YOLOv8m (medium) | 52MB | ~30ms | 50.2 | Accuracy |
| YOLOv8l (large) | 87MB | ~50ms | 52.9 | High accuracy |

> **Rekomendasi**: Gunakan **YOLOv8n** untuk real-time CV dengan ESP32-CAM.

### COCO Dataset Classes (80 kelas)
YOLO pre-trained bisa mendeteksi 80 jenis objek: person, bicycle, car, motorbike, aeroplane, bus, train, truck, boat, chair, bottle, cup, fork, knife, laptop, phone, dll.

---

## 🛠️ Praktik

### 0. Install Ultralytics

```bash
pip install ultralytics
```

### 1. Object Detection Dasar

```python
"""
sesi09_yolo_basic.py
Object detection dasar dengan YOLOv8
"""
from ultralytics import YOLO
import cv2
import imutils

# Load model (akan download otomatis saat pertama kali)
model = YOLO("yolov8n.pt")   # Nano model, cepat!

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Jalankan deteksi
    results = model(frame, verbose=False)

    # Gambar hasil deteksi
    annotated = results[0].plot()

    # Info deteksi
    num_objects = len(results[0].boxes)
    cv2.putText(annotated, f"Objects: {num_objects}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    cv2.imshow("YOLOv8 Detection", annotated)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 2. Filter Objek Tertentu

```python
"""
sesi09_yolo_filter.py
Deteksi dan filter objek tertentu saja
"""
from ultralytics import YOLO
import cv2
import imutils

model = YOLO("yolov8n.pt")

# Kelas yang ingin dideteksi (index COCO)
# 0=person, 39=bottle, 41=cup, 67=cell phone
DETECT_CLASSES = [0, 39, 41, 67]
CLASS_NAMES = {0: "Person", 39: "Bottle", 41: "Cup", 67: "Phone"}

# Warna per kelas
CLASS_COLORS = {
    0: (0, 255, 0),      # Person = hijau
    39: (255, 0, 0),     # Bottle = biru
    41: (0, 255, 255),   # Cup = kuning
    67: (255, 0, 255),   # Phone = ungu
}

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Deteksi hanya kelas tertentu
    results = model(frame, verbose=False, classes=DETECT_CLASSES)

    # Manual drawing dengan custom warna
    display = frame.copy()
    counts = {}

    for box in results[0].boxes:
        cls_id = int(box.cls[0])
        conf = float(box.conf[0])
        x1, y1, x2, y2 = map(int, box.xyxy[0])

        name = CLASS_NAMES.get(cls_id, f"Class {cls_id}")
        color = CLASS_COLORS.get(cls_id, (255, 255, 255))

        counts[name] = counts.get(name, 0) + 1

        cv2.rectangle(display, (x1, y1), (x2, y2), color, 2)
        label = f"{name}: {conf:.2f}"
        cv2.putText(display, label, (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

    # Summary
    y_pos = 30
    for name, count in counts.items():
        cv2.putText(display, f"{name}: {count}",
                    (10, y_pos), cv2.FONT_HERSHEY_SIMPLEX, 0.7,
                    (255, 255, 255), 2)
        y_pos += 30

    cv2.imshow("Filtered Detection", display)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 3. 🔨 Mini Project: People Counter → ESP32 OLED

```python
"""
sesi09_people_counter.py
🔨 MINI PROJECT: Hitung jumlah orang → tampilkan di OLED ESP32

Pipeline:
  Kamera → YOLO → count persons → Serial → ESP32 → OLED display
"""
from ultralytics import YOLO
import cv2
import imutils
import time

# Uncomment untuk Serial ESP32:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

model = YOLO("yolov8n.pt")

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

PERSON_CLASS = 0
CONFIDENCE_MIN = 0.5

last_count = -1
send_interval = 1.0  # Kirim setiap 1 detik
last_send_time = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Deteksi hanya person
    results = model(frame, verbose=False, classes=[PERSON_CLASS],
                    conf=CONFIDENCE_MIN)

    display = frame.copy()
    person_count = 0

    for box in results[0].boxes:
        conf = float(box.conf[0])
        x1, y1, x2, y2 = map(int, box.xyxy[0])
        person_count += 1

        cv2.rectangle(display, (x1, y1), (x2, y2), (0, 255, 0), 2)
        cv2.putText(display, f"Person {conf:.0%}", (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    # Kirim ke ESP32 jika berubah
    current_time = time.time()
    if person_count != last_count and current_time - last_send_time > send_interval:
        last_count = person_count
        last_send_time = current_time
        command = f"COUNT:{person_count}"
        print(f"  >> {command}")
        # ser.write(f"{command}\n".encode())

    # Overlay
    cv2.putText(display, f"People: {person_count}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    cv2.imshow("People Counter", display)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### ESP32 Firmware: OLED Display People Count

```cpp
/**
 * sesi09_oled_counter.ino
 * Tampilkan jumlah orang dari Python di OLED
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

int personCount = 0;

void setup() {
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed!");
        while (1);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("People Counter");
    display.println("Waiting...");
    display.display();
}

void updateDisplay() {
    display.clearDisplay();

    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== People Counter ===");

    // Jumlah besar di tengah
    display.setTextSize(4);
    String countStr = String(personCount);
    int16_t x = (OLED_WIDTH - countStr.length() * 24) / 2;
    display.setCursor(x > 0 ? x : 0, 20);
    display.println(personCount);

    // Footer
    display.setTextSize(1);
    display.setCursor(30, 56);
    display.println("person(s)");

    display.display();
}

void loop() {
    if (Serial.available() > 0) {
        String data = Serial.readStringUntil('\n');
        data.trim();

        // Parse "COUNT:X"
        if (data.startsWith("COUNT:")) {
            personCount = data.substring(6).toInt();
            Serial.printf("People: %d\n", personCount);
            updateDisplay();
        }
    }
}
```

---

## 📝 Latihan Mandiri

1. **Deteksi** hanya "bottle" dan "cup" — hitung jumlah masing-masing
2. **Bandingkan** performa yolov8n vs yolov8s — FPS dan akurasi
3. **Buat alert** jika jumlah orang melebihi batas (misal > 5) → buzzer ESP32
4. **Challenge**: Buat dashboard web sederhana yang menampilkan live count

---

## 📚 Referensi
- [Ultralytics YOLOv8 Docs](https://docs.ultralytics.com/)
- [YOLO Object Detection](https://pyimagesearch.com/2022/11/07/yolo-object-detection-with-opencv/)
- [COCO Dataset Classes](https://cocodataset.org/#explore)
