# Sesi 6 — Motion Detection & Background Subtraction

## 🎯 Tujuan Pembelajaran
- Mendeteksi gerakan menggunakan frame differencing
- Menggunakan Background Subtractor (MOG2, KNN)
- Membuat bounding box pada area yang bergerak
- 🔨 Mini Project: Intrusion Alert → buzzer ESP32

---

## 📖 Teori

### Frame Differencing
```
Frame(t-1)  ─┐
              ├─ abs(diff) ──► threshold ──► contour ──► bounding box
Frame(t)    ─┘
```
Sederhana tapi sensitif terhadap noise dan perubahan cahaya.

### Background Subtractor
Algoritma yang "belajar" background dan otomatis mendeteksi foreground (objek bergerak).
```
MOG2: Mixture of Gaussians — adaptif, bisa handle shadow
KNN:  K-Nearest Neighbors — bagus untuk scene statis
```

---

## 🛠️ Praktik

### 1. Frame Differencing Sederhana

```python
"""
sesi06_frame_diff.py
Deteksi gerak dengan frame differencing
"""
import cv2
import imutils

# 📷 Webcam atau ESP32-CAM
cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

ret, prev_frame = cap.read()
prev_gray = cv2.cvtColor(imutils.resize(prev_frame, width=640), cv2.COLOR_BGR2GRAY)
prev_gray = cv2.GaussianBlur(prev_gray, (21, 21), 0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    gray = cv2.GaussianBlur(gray, (21, 21), 0)

    # Hitung perbedaan frame
    diff = cv2.absdiff(prev_gray, gray)
    _, thresh = cv2.threshold(diff, 30, 255, cv2.THRESH_BINARY)

    # Bersihkan noise
    thresh = cv2.dilate(thresh, None, iterations=2)

    # Cari contour
    contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)

    motion_detected = False
    for cnt in contours:
        if cv2.contourArea(cnt) < 2000:
            continue
        motion_detected = True
        x, y, w, h = cv2.boundingRect(cnt)
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)

    status = "MOTION!" if motion_detected else "No Motion"
    color = (0, 0, 255) if motion_detected else (0, 255, 0)
    cv2.putText(frame, status, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2)

    cv2.imshow("Motion Detection", frame)
    cv2.imshow("Diff", thresh)

    prev_gray = gray.copy()

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 2. Background Subtractor (MOG2)

```python
"""
sesi06_bg_subtractor.py
Background subtraction dengan MOG2 — lebih robust
"""
import cv2
import imutils

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Buat background subtractor
bg_subtractor = cv2.createBackgroundSubtractorMOG2(
    history=500,         # Jumlah frame untuk belajar background
    varThreshold=50,     # Threshold deteksi foreground
    detectShadows=True   # Deteksi bayangan (abu-abu di mask)
)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)

    # Terapkan background subtractor
    fg_mask = bg_subtractor.apply(frame)

    # Hilangkan bayangan (nilai 127 di mask) dan noise
    _, fg_mask = cv2.threshold(fg_mask, 200, 255, cv2.THRESH_BINARY)
    fg_mask = cv2.erode(fg_mask, None, iterations=1)
    fg_mask = cv2.dilate(fg_mask, None, iterations=3)

    # Cari contour
    contours, _ = cv2.findContours(fg_mask, cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)

    result = frame.copy()
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > 3000:
            x, y, w, h = cv2.boundingRect(cnt)
            cv2.rectangle(result, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.putText(result, f"Area: {int(area)}", (x, y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

    cv2.imshow("Original", frame)
    cv2.imshow("FG Mask", fg_mask)
    cv2.imshow("Motion", result)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 3. 🔨 Mini Project: Intrusion Alert System

```python
"""
sesi06_intrusion_alert.py
🔨 MINI PROJECT: Sistem deteksi intrusi
- Deteksi gerakan dari webcam/ESP32-CAM
- Jika gerak terdeteksi → kirim BUZZ ke ESP32 → buzzer berbunyi
- Simpan screenshot saat intrusion terjadi
"""
import cv2
import imutils
import time
import os

# Uncomment salah satu untuk komunikasi ESP32:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

bg_sub = cv2.createBackgroundSubtractorMOG2(history=500, varThreshold=50)

# Konfigurasi
ALERT_AREA_MIN = 5000       # Area minimum untuk trigger alarm
COOLDOWN_SEC = 5            # Waktu antar alert (detik)
SAVE_DIR = "intrusion_logs"

os.makedirs(SAVE_DIR, exist_ok=True)

last_alert_time = 0
alert_active = False

# Tunggu beberapa frame untuk background learning
print("Learning background... Jangan bergerak selama 3 detik")
for i in range(90):  # ~3 detik pada 30fps
    ret, frame = cap.read()
    if ret:
        frame = imutils.resize(frame, width=640)
        bg_sub.apply(frame)

print("System ARMED! Monitoring...")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")

    # Background subtraction
    fg_mask = bg_sub.apply(frame)
    _, fg_mask = cv2.threshold(fg_mask, 200, 255, cv2.THRESH_BINARY)
    fg_mask = cv2.erode(fg_mask, None, iterations=1)
    fg_mask = cv2.dilate(fg_mask, None, iterations=3)

    contours, _ = cv2.findContours(fg_mask, cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)

    result = frame.copy()
    max_area = 0

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > ALERT_AREA_MIN:
            x, y, w, h = cv2.boundingRect(cnt)
            cv2.rectangle(result, (x, y), (x + w, y + h), (0, 0, 255), 2)
            max_area = max(max_area, area)

    # Check alert condition
    current_time = time.time()
    if max_area > ALERT_AREA_MIN:
        if current_time - last_alert_time > COOLDOWN_SEC:
            alert_active = True
            last_alert_time = current_time

            # Kirim alert ke ESP32
            # ser.write(b"BUZZ\n")

            # Simpan screenshot
            filename = f"{SAVE_DIR}/intrusion_{int(current_time)}.jpg"
            cv2.imwrite(filename, frame)
            print(f"⚠️  INTRUSION DETECTED! [{timestamp}] Saved: {filename}")

    elif current_time - last_alert_time > 2:
        alert_active = False

    # Overlay status
    if alert_active:
        cv2.putText(result, "!!! INTRUSION DETECTED !!!", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
        # Border merah berkedip
        if int(current_time * 2) % 2:
            cv2.rectangle(result, (0, 0),
                          (result.shape[1]-1, result.shape[0]-1), (0, 0, 255), 4)
    else:
        cv2.putText(result, f"MONITORING... [{timestamp}]", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("Intrusion Alert", result)
    cv2.imshow("Motion Mask", fg_mask)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Bandingkan** MOG2 vs KNN background subtractor — mana yang lebih baik di pencahayaan berubah?
2. **Buat zona deteksi** — hanya trigger alert jika gerak terjadi di area tertentu (ROI)
3. **Tambahkan logging** ke file CSV: timestamp, area gerak, status alert
4. **Challenge**: Buat motion heatmap — visualisasi area yang paling sering ada gerakan

---

## 📚 Referensi
- [OpenCV Background Subtraction](https://docs.opencv.org/4.x/d1/dc5/tutorial_background_subtraction.html)
- [Motion Detection Tutorial](https://pyimagesearch.com/2015/05/25/basic-motion-detection-and-tracking-with-python-and-opencv/)
