# Sesi 2 — Dasar Pengolahan Citra (Image Processing)

## 🎯 Tujuan Pembelajaran
- Memahami color spaces: BGR, RGB, HSV, Grayscale
- Menguasai teknik thresholding
- Menerapkan filter dan edge detection
- Memahami operasi morfologi

---

## 📖 Teori

### Color Spaces
```
BGR (OpenCV default)     HSV (Hue-Saturation-Value)     Grayscale
┌───┬───┬───┐            ┌───┬───┬───┐                  ┌───┐
│ B │ G │ R │            │ H │ S │ V │                  │ I │
│0-255│0-255│0-255│      │0-179│0-255│0-255│             │0-255│
└───┴───┴───┘            └───┴───┴───┘                  └───┘

H (Hue)        = Warna (0°-360° → OpenCV: 0-179)
S (Saturation) = Kepekatan warna (0=abu, 255=vivid)
V (Value)      = Kecerahan (0=gelap, 255=terang)
```

**Kenapa HSV penting?** Memisahkan informasi warna (H) dari cahaya (V), sehingga deteksi warna lebih robust terhadap perubahan pencahayaan.

### Thresholding
Mengubah gambar menjadi hitam-putih berdasarkan nilai ambang batas.
```
Piksel > threshold → Putih (255)
Piksel ≤ threshold → Hitam (0)
```

### Filter & Edge Detection
```
Blur     → Menghaluskan noise
Canny    → Mendeteksi tepi objek
Morpho   → Membersihkan noise kecil (erode/dilate)
```

---

## 🛠️ Praktik

### 1. Konversi Color Space

```python
"""
sesi02_color_space.py
Demonstrasi konversi color space
"""
import cv2
import numpy as np
import imutils

# === VERSI WEBCAM ===
cap = cv2.VideoCapture(0)
# === VERSI ESP32-CAM ===
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=500)

    # Konversi ke berbagai color space
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Pisahkan channel HSV
    h, s, v = cv2.split(hsv)

    # Tampilkan
    cv2.imshow("BGR (Original)", frame)
    cv2.imshow("Grayscale", gray)
    cv2.imshow("Hue Channel", h)
    cv2.imshow("Saturation", s)
    cv2.imshow("Value (Brightness)", v)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 2. Thresholding

```python
"""
sesi02_thresholding.py
Berbagai teknik thresholding
"""
import cv2
import imutils

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=500)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Simple threshold
    _, thresh_binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)
    _, thresh_inv = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY_INV)

    # Otsu threshold (auto-threshold)
    _, thresh_otsu = cv2.threshold(gray, 0, 255,
                                   cv2.THRESH_BINARY + cv2.THRESH_OTSU)

    # Adaptive threshold (bagus untuk pencahayaan tidak merata)
    thresh_adaptive = cv2.adaptiveThreshold(gray, 255,
                                            cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
                                            cv2.THRESH_BINARY, 11, 2)

    cv2.imshow("Original", frame)
    cv2.imshow("Binary (127)", thresh_binary)
    cv2.imshow("Otsu (Auto)", thresh_otsu)
    cv2.imshow("Adaptive", thresh_adaptive)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 3. Filter & Blur

```python
"""
sesi02_filter_blur.py
Penerapan berbagai filter
"""
import cv2
import numpy as np
import imutils

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=500)

    # Gaussian Blur (menghaluskan gambar)
    gaussian = cv2.GaussianBlur(frame, (7, 7), 0)

    # Median Blur (bagus untuk noise salt-and-pepper)
    median = cv2.medianBlur(frame, 7)

    # Bilateral Filter (menghaluskan tapi menjaga tepi)
    bilateral = cv2.bilateralFilter(frame, 9, 75, 75)

    # Sharpening (mempertajam)
    kernel_sharp = np.array([[ 0, -1,  0],
                             [-1,  5, -1],
                             [ 0, -1,  0]])
    sharpened = cv2.filter2D(frame, -1, kernel_sharp)

    cv2.imshow("Original", frame)
    cv2.imshow("Gaussian Blur", gaussian)
    cv2.imshow("Median Blur", median)
    cv2.imshow("Bilateral", bilateral)
    cv2.imshow("Sharpened", sharpened)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 4. Edge Detection (Canny)

```python
"""
sesi02_edge_detection.py
Deteksi tepi dengan Canny + trackbar untuk tuning
"""
import cv2
import imutils

def nothing(x):
    pass

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

cv2.namedWindow("Controls")
cv2.createTrackbar("Low Threshold", "Controls", 50, 255, nothing)
cv2.createTrackbar("High Threshold", "Controls", 150, 255, nothing)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=500)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)

    # Baca nilai trackbar
    low = cv2.getTrackbarPos("Low Threshold", "Controls")
    high = cv2.getTrackbarPos("High Threshold", "Controls")

    # Canny edge detection
    edges = cv2.Canny(blurred, low, high)

    cv2.imshow("Original", frame)
    cv2.imshow("Edges", edges)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 5. Operasi Morfologi

```python
"""
sesi02_morphology.py
Erode, Dilate, Opening, Closing
"""
import cv2
import numpy as np
import imutils

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=500)

    # Konversi ke biner terlebih dulu
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    _, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)

    kernel = np.ones((5, 5), np.uint8)

    # Erode: mengecilkan area putih (hilangkan noise kecil)
    eroded = cv2.erode(binary, kernel, iterations=1)

    # Dilate: memperbesar area putih (isi lubang kecil)
    dilated = cv2.dilate(binary, kernel, iterations=1)

    # Opening: erode lalu dilate (hilangkan noise)
    opening = cv2.morphologyEx(binary, cv2.MORPH_OPEN, kernel)

    # Closing: dilate lalu erode (isi lubang)
    closing = cv2.morphologyEx(binary, cv2.MORPH_CLOSE, kernel)

    cv2.imshow("Binary", binary)
    cv2.imshow("Eroded", eroded)
    cv2.imshow("Dilated", dilated)
    cv2.imshow("Opening", opening)
    cv2.imshow("Closing", closing)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Buat program** yang menampilkan semua channel HSV dari webcam secara bersamaan
2. **Eksperimen** dengan berbagai kernel size pada blur (3x3, 7x7, 15x15) — amati perbedaannya
3. **Kombinasikan** blur + Canny edge detection dan amati dampak blur terhadap hasil edge
4. **Buat program** yang otomatis saat terang/gelap dengan mengamati rata-rata nilai V channel HSV

---

## 📚 Referensi
- [OpenCV Color Spaces](https://docs.opencv.org/4.x/df/d9d/tutorial_py_colorspaces.html)
- [OpenCV Thresholding](https://docs.opencv.org/4.x/d7/d4d/tutorial_py_thresholding.html)
- [OpenCV Smoothing](https://docs.opencv.org/4.x/d4/d13/tutorial_py_filtering.html)
- [Canny Edge Detection](https://docs.opencv.org/4.x/da/d22/tutorial_py_canny.html)
- [Morphological Operations](https://docs.opencv.org/4.x/d9/d61/tutorial_py_morphological_ops.html)
