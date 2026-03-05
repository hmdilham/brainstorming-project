# Sesi 10 — Object Tracking & Counting

## 🎯 Tujuan Pembelajaran
- Memahami perbedaan detection vs tracking
- Implementasi centroid tracker sederhana
- Tracking objek melintas garis virtual (line crossing)
- 🔨 Mini Project: Smart Parking — deteksi slot kosong/terisi

---

## 📖 Teori

### Detection vs Tracking
```
Detection: Setiap frame independen      Tracking: ID persisten antar frame
 Frame 1: [A] [B]                        Frame 1: [ID1] [ID2]
 Frame 2: [?] [?]  ← siapa ini?         Frame 2: [ID1] [ID2]  ← terlacak!
 Frame 3: [?] [?] [?]                   Frame 3: [ID1] [ID2] [ID3]
```

### Centroid Tracker
Algoritma paling sederhana untuk tracking:
1. Detect objek → hitung centroid (titik tengah)
2. Bandingkan centroid frame ini dengan frame sebelumnya
3. Pasangkan berdasarkan jarak terdekat (Euclidean)
4. Jika tidak ada pasangan → objek baru / hilang

---

## 🛠️ Praktik

### 1. Centroid Tracker Class

```python
"""
utils/centroid_tracker.py
Simple centroid tracker — simpan file ini untuk digunakan ulang
"""
from collections import OrderedDict
import numpy as np
from scipy.spatial import distance


class CentroidTracker:
    def __init__(self, max_disappeared=30):
        self.next_id = 0
        self.objects = OrderedDict()       # {id: centroid}
        self.disappeared = OrderedDict()   # {id: frame_count}
        self.max_disappeared = max_disappeared

    def register(self, centroid):
        self.objects[self.next_id] = centroid
        self.disappeared[self.next_id] = 0
        self.next_id += 1

    def deregister(self, object_id):
        del self.objects[object_id]
        del self.disappeared[object_id]

    def update(self, rects):
        """
        Update tracker dengan bounding boxes baru.
        rects: list of (x1, y1, x2, y2)
        Returns: OrderedDict {id: centroid}
        """
        if len(rects) == 0:
            for oid in list(self.disappeared.keys()):
                self.disappeared[oid] += 1
                if self.disappeared[oid] > self.max_disappeared:
                    self.deregister(oid)
            return self.objects

        # Hitung centroid dari bounding boxes
        centroids = np.zeros((len(rects), 2), dtype="int")
        for i, (x1, y1, x2, y2) in enumerate(rects):
            centroids[i] = ((x1 + x2) // 2, (y1 + y2) // 2)

        if len(self.objects) == 0:
            for c in centroids:
                self.register(c)
        else:
            object_ids = list(self.objects.keys())
            object_centroids = list(self.objects.values())

            # Hitung jarak antara existing dan new centroids
            D = distance.cdist(np.array(object_centroids), centroids)

            # Pasangkan berdasarkan jarak terdekat
            rows = D.min(axis=1).argsort()
            cols = D.argmin(axis=1)[rows]

            used_rows, used_cols = set(), set()

            for (row, col) in zip(rows, cols):
                if row in used_rows or col in used_cols:
                    continue
                if D[row, col] > 80:  # Max distance threshold
                    continue

                oid = object_ids[row]
                self.objects[oid] = centroids[col]
                self.disappeared[oid] = 0
                used_rows.add(row)
                used_cols.add(col)

            # Objek yang hilang
            unused_rows = set(range(D.shape[0])) - used_rows
            unused_cols = set(range(D.shape[1])) - used_cols

            for row in unused_rows:
                oid = object_ids[row]
                self.disappeared[oid] += 1
                if self.disappeared[oid] > self.max_disappeared:
                    self.deregister(oid)

            # Objek baru
            for col in unused_cols:
                self.register(centroids[col])

        return self.objects
```

### 2. Object Tracking dengan YOLO + Centroid Tracker

```python
"""
sesi10_tracking.py
Object tracking menggunakan YOLO detection + centroid tracker
"""
from ultralytics import YOLO
import cv2
import imutils
import sys
import os
sys.path.append(os.path.dirname(__file__) + "/..")
from utils.centroid_tracker import CentroidTracker

model = YOLO("yolov8n.pt")
tracker = CentroidTracker(max_disappeared=40)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Warna unik per ID
COLORS = [(255,0,0), (0,255,0), (0,0,255), (255,255,0), (255,0,255),
          (0,255,255), (128,255,0), (255,128,0), (128,0,255)]

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Detect persons
    results = model(frame, verbose=False, classes=[0], conf=0.4)

    # Extract bounding boxes
    rects = []
    for box in results[0].boxes:
        x1, y1, x2, y2 = map(int, box.xyxy[0])
        rects.append((x1, y1, x2, y2))

    # Update tracker
    objects = tracker.update(rects)

    # Draw tracked objects
    display = frame.copy()
    for (oid, centroid) in objects.items():
        color = COLORS[oid % len(COLORS)]
        cx, cy = centroid

        cv2.circle(display, (cx, cy), 6, color, -1)
        cv2.putText(display, f"ID:{oid}", (cx - 20, cy - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, color, 2)

    # Draw bounding boxes
    for (x1, y1, x2, y2) in rects:
        cv2.rectangle(display, (x1, y1), (x2, y2), (200, 200, 200), 1)

    cv2.putText(display, f"Tracking: {len(objects)} objects", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    cv2.imshow("Object Tracking", display)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 3. Line Crossing Counter

```python
"""
sesi10_line_crossing.py
Menghitung objek yang melewati garis virtual
"""
from ultralytics import YOLO
import cv2
import imutils
import numpy as np
import sys, os
sys.path.append(os.path.dirname(__file__) + "/..")
from utils.centroid_tracker import CentroidTracker

model = YOLO("yolov8n.pt")
tracker = CentroidTracker(max_disappeared=40)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Garis virtual (horizontal di tengah frame)
LINE_Y = 300
OFFSET = 10  # Toleransi

count_in = 0
count_out = 0
prev_positions = {}  # {id: last_y_position}

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    h, w = frame.shape[:2]
    LINE_Y = h // 2

    # Detect
    results = model(frame, verbose=False, classes=[0], conf=0.4)
    rects = []
    for box in results[0].boxes:
        x1, y1, x2, y2 = map(int, box.xyxy[0])
        rects.append((x1, y1, x2, y2))

    # Track
    objects = tracker.update(rects)

    display = frame.copy()

    # Gambar garis virtual
    cv2.line(display, (0, LINE_Y), (w, LINE_Y), (0, 255, 255), 2)
    cv2.putText(display, "COUNTING LINE", (10, LINE_Y - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)

    for (oid, centroid) in objects.items():
        cx, cy = centroid
        cv2.circle(display, (cx, cy), 5, (0, 255, 0), -1)
        cv2.putText(display, f"ID:{oid}", (cx - 20, cy - 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

        # Check line crossing
        if oid in prev_positions:
            prev_y = prev_positions[oid]

            # Melewati dari atas ke bawah
            if prev_y < LINE_Y - OFFSET and cy >= LINE_Y - OFFSET:
                count_in += 1
                print(f"IN: ID {oid} (total IN={count_in})")

            # Melewati dari bawah ke atas
            elif prev_y > LINE_Y + OFFSET and cy <= LINE_Y + OFFSET:
                count_out += 1
                print(f"OUT: ID {oid} (total OUT={count_out})")

        prev_positions[oid] = cy

    # Bersihkan ID yang sudah hilang
    active_ids = set(objects.keys())
    prev_positions = {k: v for k, v in prev_positions.items() if k in active_ids}

    # Info
    cv2.putText(display, f"IN: {count_in}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)
    cv2.putText(display, f"OUT: {count_out}", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
    cv2.putText(display, f"Inside: {count_in - count_out}", (10, 90),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 0), 2)

    cv2.imshow("Line Crossing Counter", display)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 4. 🔨 Mini Project: Smart Parking

```python
"""
sesi10_smart_parking.py
🔨 MINI PROJECT: Deteksi slot parkir kosong/terisi
Simulasi: definisikan ROI untuk setiap slot parkir di kamera
"""
import cv2
import imutils
import numpy as np

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Definisikan slot parkir sebagai region of interest
# Format: (x, y, w, h) — sesuaikan dengan posisi kamera Anda
PARKING_SLOTS = [
    {"id": 1, "rect": (50,  200, 120, 180)},
    {"id": 2, "rect": (190, 200, 120, 180)},
    {"id": 3, "rect": (330, 200, 120, 180)},
    {"id": 4, "rect": (470, 200, 120, 180)},
]

# Threshold: jumlah piksel putih untuk "terisi"
OCCUPIED_THRESHOLD = 3000

# Background subtractor
bg_sub = cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=40)

# Learning phase
print("Learning background... Park kosong dulu")
for i in range(100):
    ret, frame = cap.read()
    if ret:
        frame = imutils.resize(frame, width=640)
        bg_sub.apply(frame)
print("System ready!")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Background subtraction
    fg_mask = bg_sub.apply(frame, learningRate=0.001)
    _, fg_mask = cv2.threshold(fg_mask, 200, 255, cv2.THRESH_BINARY)
    fg_mask = cv2.dilate(fg_mask, None, iterations=2)

    display = frame.copy()
    free_slots = 0
    total_slots = len(PARKING_SLOTS)

    for slot in PARKING_SLOTS:
        x, y, w, h = slot["rect"]
        sid = slot["id"]

        # Cek apakah slot terisi
        roi = fg_mask[y:y+h, x:x+w]
        white_pixels = cv2.countNonZero(roi)

        occupied = white_pixels > OCCUPIED_THRESHOLD

        if occupied:
            color = (0, 0, 255)    # Merah = terisi
            label = f"Slot {sid}: FULL"
        else:
            color = (0, 255, 0)    # Hijau = kosong
            label = f"Slot {sid}: FREE"
            free_slots += 1

        cv2.rectangle(display, (x, y), (x + w, y + h), color, 2)
        cv2.putText(display, label, (x, y - 5),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

    # Summary
    cv2.putText(display, f"Free: {free_slots}/{total_slots}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

    cv2.imshow("Smart Parking", display)
    cv2.imshow("Motion Mask", fg_mask)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Install scipy** (`pip install scipy`) dan test centroid tracker
2. **Tracking** dengan YOLO: track bottle atau cup yang bergerak di meja
3. **Modifikasi** line crossing counter untuk mengirim data ke ESP32 OLED
4. **Challenge**: Buat dashboard web real-time untuk smart parking

---

## 📚 Referensi
- [Centroid Tracking](https://pyimagesearch.com/2018/07/23/simple-object-tracking-with-opencv/)
- [SORT Algorithm](https://github.com/abewley/sort)
- [People Counting](https://pyimagesearch.com/2018/08/13/opencv-people-counter/)
