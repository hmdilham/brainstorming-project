# Sesi 1 — Pengenalan Computer Vision & Setup Environment

## 🎯 Tujuan Pembelajaran
- Memahami apa itu Computer Vision dan perbedaannya dengan Image Processing
- Menyiapkan environment Python + OpenCV
- Melakukan operasi dasar pada gambar: baca, tampilkan, manipulasi
- Memahami cara kerja webcam sebagai input kamera

---

## 📖 Teori

### Apa itu Computer Vision?
Computer Vision (CV) adalah cabang AI yang memungkinkan komputer "melihat" dan memahami konten visual (gambar/video).

```
Image Processing     →  Mengolah piksel (filter, resize, rotate)
Computer Vision      →  Memahami konten (ini wajah, ini mobil)
AI Vision            →  Mengambil keputusan dari pemahaman visual
```

### Bagaimana Komputer Melihat Gambar?
- Gambar = matriks/array 2D (grayscale) atau 3D (color)
- Setiap piksel = nilai intensitas cahaya (0–255)
- Gambar berwarna = 3 channel: **Blue, Green, Red** (BGR di OpenCV)

```
Gambar 3x3 Grayscale:        Gambar 3x3 BGR (3 channel):
┌─────┬─────┬─────┐          ┌─────┬─────┬─────┐
│ 120 │ 200 │  50 │          │B,G,R│B,G,R│B,G,R│
├─────┼─────┼─────┤          ├─────┼─────┼─────┤
│  80 │ 255 │ 100 │          │B,G,R│B,G,R│B,G,R│
├─────┼─────┼─────┤          ├─────┼─────┼─────┤
│  30 │  10 │ 180 │          │B,G,R│B,G,R│B,G,R│
└─────┴─────┴─────┘          └─────┴─────┴─────┘
        h x w                      h x w x 3
```

### Library yang Digunakan
| Library | Fungsi |
|---------|--------|
| `opencv-python` | Pengolahan citra & video |
| `numpy` | Operasi array/matriks |
| `imutils` | Helper function (resize, rotate, dll) |

---

## 🛠️ Praktik

### 1. Setup Environment

```bash
# Buat virtual environment
python -m venv cv-env
source cv-env/bin/activate    # Linux/Mac
# cv-env\Scripts\activate     # Windows

# Install dependencies
pip install opencv-python numpy imutils
```

**Verifikasi instalasi:**
```python
import cv2
import numpy as np

print(f"OpenCV version: {cv2.__version__}")
print(f"NumPy version: {np.__version__}")
```

### 2. Baca & Tampilkan Gambar

```python
"""
sesi01_baca_gambar.py
Membaca dan menampilkan gambar dari file
"""
import cv2

# Baca gambar dari file
img = cv2.imread("contoh.jpg")

# Cek apakah berhasil dibaca
if img is None:
    print("Error: gambar tidak ditemukan!")
    exit()

# Info gambar
print(f"Dimensi: {img.shape}")        # (height, width, channels)
print(f"Tipe data: {img.dtype}")       # uint8
print(f"Total piksel: {img.size}")     # h * w * channels

# Tampilkan gambar
cv2.imshow("Gambar Asli", img)
cv2.waitKey(0)           # Tunggu sampai tombol ditekan
cv2.destroyAllWindows()
```

### 3. Manipulasi Dasar Gambar

```python
"""
sesi01_manipulasi_dasar.py
Resize, crop, rotate, flip gambar
"""
import cv2
import imutils

img = cv2.imread("contoh.jpg")

# === RESIZE ===
# Resize ke ukuran tertentu
resized = cv2.resize(img, (400, 300))

# Resize dengan mempertahankan aspect ratio
resized_ratio = imutils.resize(img, width=400)

# === CROP ===
# Crop bagian tertentu [y1:y2, x1:x2]
h, w = img.shape[:2]
cropped = img[50:200, 100:300]

# === ROTATE ===
# Rotate 90 derajat
rotated_90 = cv2.rotate(img, cv2.ROTATE_90_CLOCKWISE)

# Rotate dengan sudut bebas
rotated_45 = imutils.rotate(img, angle=45)

# Rotate tanpa crop (bound)
rotated_bound = imutils.rotate_bound(img, angle=45)

# === FLIP ===
flip_h = cv2.flip(img, 1)    # Horizontal (mirror)
flip_v = cv2.flip(img, 0)    # Vertical
flip_both = cv2.flip(img, -1) # Both

# Tampilkan semua hasil
cv2.imshow("Original", imutils.resize(img, width=300))
cv2.imshow("Resized", resized)
cv2.imshow("Cropped", cropped)
cv2.imshow("Rotated 45", imutils.resize(rotated_45, width=300))
cv2.imshow("Flip Horizontal", imutils.resize(flip_h, width=300))

cv2.waitKey(0)
cv2.destroyAllWindows()
```

### 4. Menggambar pada Gambar (Drawing)

```python
"""
sesi01_drawing.py
Menggambar bentuk dan teks pada gambar
"""
import cv2
import numpy as np

# Buat canvas hitam 500x500
canvas = np.zeros((500, 500, 3), dtype=np.uint8)

# Garis
cv2.line(canvas, (50, 50), (450, 50), (0, 255, 0), 2)

# Persegi
cv2.rectangle(canvas, (50, 100), (200, 250), (255, 0, 0), 2)
cv2.rectangle(canvas, (250, 100), (400, 250), (0, 0, 255), -1)  # filled

# Lingkaran
cv2.circle(canvas, (250, 380), 80, (0, 255, 255), 2)
cv2.circle(canvas, (250, 380), 40, (255, 0, 255), -1)  # filled

# Teks
cv2.putText(canvas, "Hello OpenCV!", (100, 470),
            cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

cv2.imshow("Drawing", canvas)
cv2.waitKey(0)
cv2.destroyAllWindows()
```

### 5. Membaca Video dari Webcam

```python
"""
sesi01_webcam.py
Membaca video stream dari webcam laptop
"""
import cv2
import imutils

# 📷 MODE WEBCAM — gunakan kamera laptop
cap = cv2.VideoCapture(0)

# Cek apakah kamera terbuka
if not cap.isOpened():
    print("Error: tidak bisa membuka kamera!")
    exit()

print("Tekan 'q' untuk keluar, 's' untuk screenshot")

frame_count = 0
while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: gagal membaca frame!")
        break

    frame_count += 1

    # Resize agar konsisten
    frame = imutils.resize(frame, width=640)

    # Tambahkan info di frame
    h, w = frame.shape[:2]
    cv2.putText(frame, f"Frame: {frame_count} | Size: {w}x{h}",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # Mirror (agar natural seperti cermin)
    frame = cv2.flip(frame, 1)

    cv2.imshow("Webcam", frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('s'):
        filename = f"screenshot_{frame_count}.jpg"
        cv2.imwrite(filename, frame)
        print(f"Screenshot disimpan: {filename}")

cap.release()
cv2.destroyAllWindows()
```

### 6. [ESP32-CAM] Membaca Stream dari ESP32-CAM

> ⚠️ **Catatan**: Bagian ini memerlukan ESP32-CAM yang sudah di-flash firmware CameraWebServer (akan dibahas detail di Sesi 4). Di sini hanya ditunjukkan cara membaca stream-nya dari Python.

```python
"""
sesi01_esp32cam_stream.py
Membaca video stream dari ESP32-CAM
(Memerlukan ESP32-CAM sudah aktif streaming)
"""
import cv2
import imutils

# 📡 MODE ESP32-CAM — ganti IP sesuai ESP32-CAM Anda
ESP32_CAM_URL = "http://192.168.1.100:81/stream"

cap = cv2.VideoCapture(ESP32_CAM_URL)

if not cap.isOpened():
    print("Error: tidak bisa konek ke ESP32-CAM!")
    print("Pastikan ESP32-CAM aktif dan IP benar")
    exit()

print("Streaming dari ESP32-CAM... Tekan 'q' untuk keluar")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Frame lost, reconnecting...")
        cap = cv2.VideoCapture(ESP32_CAM_URL)
        continue

    frame = imutils.resize(frame, width=640)

    cv2.putText(frame, "ESP32-CAM Stream", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("ESP32-CAM", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 7. Helper: Fungsi Utilitas yang Akan Sering Dipakai

```python
"""
utils/camera.py
Utility functions untuk membuka kamera (webcam atau ESP32-CAM)
Simpan file ini, akan digunakan di sesi-sesi berikutnya
"""
import cv2
import imutils


def open_camera(source=0, width=640):
    """
    Buka kamera dari webcam atau ESP32-CAM.

    Args:
        source: 0 untuk webcam, atau URL string untuk ESP32-CAM
                contoh: "http://192.168.1.100:81/stream"
        width: lebar frame output

    Returns:
        cv2.VideoCapture object
    """
    cap = cv2.VideoCapture(source)
    if not cap.isOpened():
        raise Exception(f"Tidak bisa membuka kamera: {source}")
    return cap


def read_frame(cap, width=640, mirror=True):
    """
    Baca satu frame dari kamera.

    Returns:
        tuple: (success, frame)
    """
    ret, frame = cap.read()
    if not ret:
        return False, None

    frame = imutils.resize(frame, width=width)
    if mirror:
        frame = cv2.flip(frame, 1)

    return True, frame


# ============================================
# Contoh penggunaan:
# ============================================
if __name__ == "__main__":
    # Pilih salah satu:
    # cap = open_camera(0)                                   # Webcam
    # cap = open_camera("http://192.168.1.100:81/stream")    # ESP32-CAM

    cap = open_camera(0)  # Default: webcam

    while True:
        ok, frame = read_frame(cap)
        if not ok:
            break

        cv2.imshow("Camera", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Baca gambar** yang Anda punya dan tampilkan info dimensinya
2. **Crop wajah** dari sebuah foto dan simpan sebagai file baru
3. **Buat canvas** dan gambar rumah sederhana dari bentuk-bentuk geometri
4. **Buka webcam** dan tambahkan overlay teks + kotak di tengah frame
5. **Modifikasi** `utils/camera.py` agar bisa switch antara webcam dan ESP32-CAM dengan argumen command line

---

## 📚 Referensi
- [OpenCV Python Documentation](https://docs.opencv.org/4.x/d6/d00/tutorial_py_root.html)
- [OpenCV Getting Started](https://docs.opencv.org/4.x/d9/df8/tutorial_root.html)
- [NumPy for Image Processing](https://numpy.org/doc/stable/user/absolute_beginners.html)
