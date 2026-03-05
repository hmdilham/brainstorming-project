# Sesi 19 — Custom Model Training (YOLOv8)

## 🎯 Tujuan Pembelajaran
- Mengumpulkan dan melabeli dataset custom
- Training YOLOv8-nano dengan transfer learning
- Deploy dan test custom model
- 🔨 Mini Project: Helmet Detector (APD Safety)

---

## 📖 Teori

### Pipeline Training Custom YOLO
```
1. Collect    →  2. Label     →  3. Train    →  4. Deploy
  Kumpulkan      Beri label       YOLOv8         Test di
  gambar         (Roboflow)       transfer       kamera
  (100-500)                       learning
```

### Berapa Banyak Gambar yang Dibutuhkan?
| Level | Jumlah/Kelas | Akurasi |
|-------|:------------:|:-------:|
| Minimal | 50-100 | ⭐⭐ |
| Cukup | 200-500 | ⭐⭐⭐ |
| Bagus | 500-1000 | ⭐⭐⭐⭐ |
| Excellent | 1000+ | ⭐⭐⭐⭐⭐ |

> 💡 Dengan **transfer learning**, 100-200 gambar per kelas sudah bisa menghasilkan model yang cukup baik.

### Dataset Structure
```
dataset/
├── train/
│   ├── images/
│   │   ├── img001.jpg
│   │   └── img002.jpg
│   └── labels/
│       ├── img001.txt
│       └── img002.txt
├── valid/
│   ├── images/
│   └── labels/
└── data.yaml
```

### YOLO Label Format
Setiap file `.txt` berisi:
```
class_id  center_x  center_y  width  height
0         0.45      0.32      0.15   0.20
```
(Semua nilai dalam range 0-1, relatif terhadap ukuran gambar)

---

## 🛠️ Praktik

### 1. Kumpulkan Dataset dari Kamera

```python
"""
sesi19_collect_dataset.py
Kumpulkan gambar untuk training dari kamera
"""
import cv2
import imutils
import os
import time

SAVE_DIR = "dataset_raw"
os.makedirs(SAVE_DIR, exist_ok=True)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

count = len(os.listdir(SAVE_DIR))

print("=== Dataset Collector ===")
print("Tekan 'c' untuk capture gambar")
print("Tekan 'a' untuk auto-capture (setiap 0.5 detik)")
print("Tekan 's' untuk stop auto-capture")
print("Tekan 'q' untuk keluar")

auto_capture = False
last_auto_time = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Auto capture
    if auto_capture:
        now = time.time()
        if now - last_auto_time > 0.5:
            filename = f"{SAVE_DIR}/img_{count:04d}.jpg"
            cv2.imwrite(filename, frame)
            count += 1
            last_auto_time = now

    mode = "AUTO" if auto_capture else "MANUAL"
    cv2.putText(frame, f"[{mode}] Captured: {count}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("Dataset Collector", frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('c'):
        filename = f"{SAVE_DIR}/img_{count:04d}.jpg"
        cv2.imwrite(filename, frame)
        count += 1
        print(f"Saved: {filename}")
    elif key == ord('a'):
        auto_capture = True
        print("Auto-capture ON")
    elif key == ord('s'):
        auto_capture = False
        print("Auto-capture OFF")

print(f"\nTotal images: {count}")
cap.release()
cv2.destroyAllWindows()
```

### 2. Labeling dengan Roboflow (Online) atau LabelImg (Offline)

#### Opsi A: Roboflow (Recommended)
```
1. Buka https://roboflow.com/ → Create Free Account
2. New Project → Object Detection
3. Upload gambar dari folder dataset_raw/
4. Annotate (gambar bounding box + beri label)
5. Generate → Export as "YOLOv8 format"
6. Download dataset (sudah split train/val)
```

#### Opsi B: LabelImg (Offline)
```bash
pip install labelImg
labelImg dataset_raw/
```
- Set format YOLO
- Gambar bounding box di setiap gambar
- Simpan label ke folder `labels/`

### 3. Prepare data.yaml

```yaml
# dataset/data.yaml
path: /absolute/path/to/dataset
train: train/images
val: valid/images

nc: 2  # Jumlah kelas
names: ['helmet', 'no_helmet']
```

### 4. Training YOLOv8

```python
"""
sesi19_train_yolo.py
Training YOLOv8 custom model
"""
from ultralytics import YOLO

# Load pre-trained model (nano untuk kecepatan)
model = YOLO("yolov8n.pt")

# Training
results = model.train(
    data="dataset/data.yaml",   # Path ke data.yaml
    epochs=50,                   # Jumlah epoch
    imgsz=640,                   # Size gambar
    batch=16,                    # Batch size (kurangi jika GPU kecil)
    patience=10,                 # Early stopping
    name="helmet_detector",      # Nama experiment
    device="0",                  # GPU index (atau "cpu")
    workers=4,
    pretrained=True,             # Transfer learning
    verbose=True,
)

# Evaluate
metrics = model.val()
print(f"mAP50: {metrics.box.map50:.3f}")
print(f"mAP50-95: {metrics.box.map:.3f}")

# Export ke format ringan (opsional)
# model.export(format="onnx")
# model.export(format="tflite")

print(f"\n✅ Model saved to: runs/detect/helmet_detector/weights/best.pt")
```

### 5. 🔨 Mini Project: Helmet Detector

```python
"""
sesi19_helmet_detector.py
🔨 MINI PROJECT: Deteksi apakah pekerja memakai helm
Jika tidak → alert ke ESP32
"""
from ultralytics import YOLO
import cv2
import imutils
import time

# Load custom model
MODEL_PATH = "runs/detect/helmet_detector/weights/best.pt"
model = YOLO(MODEL_PATH)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Uncomment untuk Serial:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

CLASS_NAMES = {0: "Helmet", 1: "No Helmet"}
CLASS_COLORS = {0: (0, 255, 0), 1: (0, 0, 255)}

last_alert_time = 0
ALERT_COOLDOWN = 5

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Detect
    results = model(frame, verbose=False, conf=0.4)

    display = frame.copy()
    no_helmet_detected = False

    for box in results[0].boxes:
        cls_id = int(box.cls[0])
        conf = float(box.conf[0])
        x1, y1, x2, y2 = map(int, box.xyxy[0])

        name = CLASS_NAMES.get(cls_id, f"Class {cls_id}")
        color = CLASS_COLORS.get(cls_id, (255, 255, 255))

        cv2.rectangle(display, (x1, y1), (x2, y2), color, 2)
        cv2.putText(display, f"{name} {conf:.0%}", (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

        if cls_id == 1:  # No helmet
            no_helmet_detected = True

    # Alert
    now = time.time()
    if no_helmet_detected and now - last_alert_time > ALERT_COOLDOWN:
        last_alert_time = now
        print("⚠️ WORKER WITHOUT HELMET DETECTED!")
        # ser.write(b"SAFETY_ALERT\n")

        cv2.putText(display, "!!! NO HELMET !!!", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
    else:
        cv2.putText(display, "Safety: OK" if not no_helmet_detected else "WARNING!",
                    (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8,
                    (0, 255, 0) if not no_helmet_detected else (0, 0, 255), 2)

    cv2.imshow("Helmet Detector", display)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Kumpulkan 100 gambar** objek yang ingin Anda deteksi
2. **Labeling** menggunakan Roboflow → export YOLO format
3. **Train** YOLOv8n selama 50 epoch, lihat mAP-nya
4. **Test** model custom dengan webcam dan ESP32-CAM stream
5. **Challenge**: Train model untuk mendeteksi tools (obeng, tang, palu)

---

## 📚 Referensi
- [Ultralytics Train Docs](https://docs.ultralytics.com/modes/train/)
- [Roboflow](https://roboflow.com/)
- [LabelImg](https://github.com/heartexlabs/labelImg)
- [YOLOv8 Custom Training Tutorial](https://docs.ultralytics.com/datasets/)
