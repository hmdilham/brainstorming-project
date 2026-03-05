# Sesi 3 — Contour, Shape Detection & Color Detection

## 🎯 Tujuan Pembelajaran
- Mendeteksi dan menggambar contour objek
- Mengenali bentuk geometri (lingkaran, persegi, segitiga)
- Mendeteksi objek berdasarkan warna menggunakan HSV masking
- Membuat trackbar untuk tuning HSV range secara real-time

---

## 📖 Teori

### Contour
Contour = kurva yang menghubungkan semua titik dengan intensitas yang sama di tepian objek.
```
Gambar biner  →  findContours()  →  Daftar titik-titik kontur
                      ↓
              drawContours()  →  Gambar kontur di frame
```

### Shape Detection
Setiap bentuk geometri memiliki jumlah sudut (vertex) berbeda:
```
Segitiga  = 3 vertex
Persegi   = 4 vertex (+ cek rasio w/h ≈ 1)
Rectangle = 4 vertex (rasio w/h ≠ 1)
Lingkaran = >8 vertex (atau gunakan circularity)
```

### HSV Color Range untuk Warna Umum
| Warna | H Min | H Max | S Min | V Min |
|-------|:-----:|:-----:|:-----:|:-----:|
| Merah | 0-10 atau 170-179 | 10 atau 179 | 100 | 100 |
| Hijau | 35 | 85 | 100 | 100 |
| Biru | 100 | 130 | 100 | 100 |
| Kuning | 20 | 35 | 100 | 100 |
| Oranye | 10 | 20 | 100 | 100 |

---

## 🛠️ Praktik

### 1. Deteksi Contour

```python
"""
sesi03_contour.py
Mendeteksi dan menggambar contour dari kamera
"""
import cv2
import imutils

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)

    # Threshold
    _, binary = cv2.threshold(blurred, 100, 255, cv2.THRESH_BINARY_INV)

    # Cari contour
    contours, hierarchy = cv2.findContours(binary, cv2.RETR_EXTERNAL,
                                           cv2.CHAIN_APPROX_SIMPLE)

    # Gambar semua contour
    result = frame.copy()
    cv2.drawContours(result, contours, -1, (0, 255, 0), 2)

    # Info
    cv2.putText(result, f"Contours found: {len(contours)}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # Untuk setiap contour, tampilkan bounding box
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > 500:  # Filter noise
            x, y, w, h = cv2.boundingRect(cnt)
            cv2.rectangle(result, (x, y), (x + w, y + h), (255, 0, 0), 2)

            # Tampilkan area
            cv2.putText(result, f"A:{int(area)}", (x, y - 5),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 0, 0), 1)

    cv2.imshow("Original", frame)
    cv2.imshow("Binary", binary)
    cv2.imshow("Contours", result)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 2. Shape Detection (Deteksi Bentuk)

```python
"""
sesi03_shape_detection.py
Mendeteksi bentuk geometri dari kamera
Gunakan kertas putih dengan bentuk-bentuk berwarna gelap
"""
import cv2
import imutils

def detect_shape(contour):
    """Identifikasi bentuk berdasarkan jumlah vertex."""
    peri = cv2.arcLength(contour, True)
    approx = cv2.approxPolyDP(contour, 0.04 * peri, True)
    vertices = len(approx)

    if vertices == 3:
        return "Segitiga"
    elif vertices == 4:
        # Cek apakah persegi atau rectangle
        x, y, w, h = cv2.boundingRect(approx)
        ratio = w / float(h)
        if 0.85 <= ratio <= 1.15:
            return "Persegi"
        else:
            return "Rectangle"
    elif vertices == 5:
        return "Pentagon"
    elif vertices > 5:
        # Cek circularity
        area = cv2.contourArea(contour)
        circularity = (4 * 3.14159 * area) / (peri * peri)
        if circularity > 0.7:
            return "Lingkaran"
        return f"Polygon({vertices})"
    return "Unknown"


cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    _, binary = cv2.threshold(blurred, 100, 255, cv2.THRESH_BINARY_INV)

    contours, _ = cv2.findContours(binary, cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)

    result = frame.copy()
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > 1000:
            shape = detect_shape(cnt)

            # Gambar contour dan label
            cv2.drawContours(result, [cnt], -1, (0, 255, 0), 2)

            # Centroid untuk posisi label
            M = cv2.moments(cnt)
            if M["m00"] > 0:
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])
                cv2.putText(result, shape, (cx - 30, cy),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

    cv2.imshow("Shape Detection", result)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 3. Color Detection dengan HSV Masking

```python
"""
sesi03_color_detection.py
Deteksi objek berwarna tertentu menggunakan HSV masking
"""
import cv2
import numpy as np
import imutils

# Definisikan range warna dalam HSV
COLOR_RANGES = {
    "Merah": [
        (np.array([0, 100, 100]), np.array([10, 255, 255])),
        (np.array([170, 100, 100]), np.array([179, 255, 255]))
    ],
    "Hijau": [
        (np.array([35, 100, 100]), np.array([85, 255, 255]))
    ],
    "Biru": [
        (np.array([100, 100, 100]), np.array([130, 255, 255]))
    ],
    "Kuning": [
        (np.array([20, 100, 100]), np.array([35, 255, 255]))
    ],
}

# Warna BGR untuk drawing (sesuai nama warna)
DRAW_COLORS = {
    "Merah": (0, 0, 255),
    "Hijau": (0, 255, 0),
    "Biru": (255, 0, 0),
    "Kuning": (0, 255, 255),
}

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Blur untuk mengurangi noise
    hsv = cv2.GaussianBlur(hsv, (11, 11), 0)

    result = frame.copy()

    for color_name, ranges in COLOR_RANGES.items():
        # Gabungkan mask jika ada multiple range (misal merah)
        mask = np.zeros(hsv.shape[:2], dtype=np.uint8)
        for (lower, upper) in ranges:
            mask = cv2.bitwise_or(mask, cv2.inRange(hsv, lower, upper))

        # Bersihkan mask
        mask = cv2.erode(mask, None, iterations=2)
        mask = cv2.dilate(mask, None, iterations=2)

        # Cari contour pada mask
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL,
                                        cv2.CHAIN_APPROX_SIMPLE)

        for cnt in contours:
            area = cv2.contourArea(cnt)
            if area > 800:
                x, y, w, h = cv2.boundingRect(cnt)
                draw_color = DRAW_COLORS.get(color_name, (255, 255, 255))

                cv2.rectangle(result, (x, y), (x + w, y + h), draw_color, 2)
                cv2.putText(result, color_name, (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, draw_color, 2)

    cv2.imshow("Color Detection", result)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 4. HSV Trackbar (Tuning Tool)

```python
"""
sesi03_hsv_trackbar.py
Tool interaktif untuk menemukan HSV range warna tertentu
Sangat berguna untuk menemukan range warna yang tepat!
"""
import cv2
import numpy as np
import imutils

def nothing(x):
    pass

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

cv2.namedWindow("HSV Trackbar")
cv2.createTrackbar("H Min", "HSV Trackbar", 0, 179, nothing)
cv2.createTrackbar("H Max", "HSV Trackbar", 179, 179, nothing)
cv2.createTrackbar("S Min", "HSV Trackbar", 50, 255, nothing)
cv2.createTrackbar("S Max", "HSV Trackbar", 255, 255, nothing)
cv2.createTrackbar("V Min", "HSV Trackbar", 50, 255, nothing)
cv2.createTrackbar("V Max", "HSV Trackbar", 255, 255, nothing)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Baca trackbar
    h_min = cv2.getTrackbarPos("H Min", "HSV Trackbar")
    h_max = cv2.getTrackbarPos("H Max", "HSV Trackbar")
    s_min = cv2.getTrackbarPos("S Min", "HSV Trackbar")
    s_max = cv2.getTrackbarPos("S Max", "HSV Trackbar")
    v_min = cv2.getTrackbarPos("V Min", "HSV Trackbar")
    v_max = cv2.getTrackbarPos("V Max", "HSV Trackbar")

    lower = np.array([h_min, s_min, v_min])
    upper = np.array([h_max, s_max, v_max])

    # Buat mask
    mask = cv2.inRange(hsv, lower, upper)
    result = cv2.bitwise_and(frame, frame, mask=mask)

    # Tampilkan range yang sedang dipakai
    range_text = f"H:[{h_min}-{h_max}] S:[{s_min}-{s_max}] V:[{v_min}-{v_max}]"
    cv2.putText(result, range_text, (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    cv2.imshow("Original", frame)
    cv2.imshow("Mask", mask)
    cv2.imshow("HSV Trackbar", result)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('p'):
        # Print range ke terminal (untuk dicatat)
        print(f"lower = np.array([{h_min}, {s_min}, {v_min}])")
        print(f"upper = np.array([{h_max}, {s_max}, {v_max}])")
        print("---")

cap.release()
cv2.destroyAllWindows()
```

> 💡 **Tips**: Tekan `p` untuk print HSV range yang sudah Anda temukan ke terminal!

---

## 📝 Latihan Mandiri

1. **Deteksi dan hitung** berapa objek merah, hijau, dan biru yang terlihat di kamera
2. **Gunakan HSV Trackbar** untuk menemukan range warna kulit tangan Anda
3. **Kombinasikan** shape detection + color detection: deteksi "lingkaran merah" atau "persegi biru"
4. **Buat program** yang mendeteksi bola warna dan menampilkan posisi X,Y-nya di terminal

---

## 📚 Referensi
- [OpenCV Contours](https://docs.opencv.org/4.x/d4/d73/tutorial_py_contours_begin.html)
- [Shape Detection OpenCV](https://docs.opencv.org/4.x/dd/d49/tutorial_py_contour_features.html)
- [Color Detection HSV](https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html)
