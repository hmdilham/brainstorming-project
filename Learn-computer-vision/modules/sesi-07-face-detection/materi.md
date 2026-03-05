# Sesi 7 — Face Detection (Haar Cascade & DNN)

## 🎯 Tujuan Pembelajaran
- Mendeteksi wajah menggunakan Haar Cascade Classifier
- Mendeteksi wajah menggunakan DNN (Deep Neural Network)
- Membandingkan akurasi dan kecepatan kedua metode
- Crop dan simpan wajah yang terdeteksi

---

## 📖 Teori

### Perbandingan Metode Face Detection
| Metode | Kecepatan | Akurasi | Rotasi | Profile |
|--------|:---------:|:-------:|:------:|:-------:|
| Haar Cascade | ⚡ Cepat | ⭐⭐ | ❌ | ❌ |
| DNN (Caffe) | ⚡ Cepat | ⭐⭐⭐⭐ | ✅ | ✅ |
| MediaPipe | ⚡ Cepat | ⭐⭐⭐⭐⭐ | ✅ | ✅ |

### Haar Cascade
- Menggunakan fitur Haar (perbedaan intensitas area gelap/terang)
- Sudah built-in di OpenCV
- Cepat, tapi kurang akurat untuk wajah miring/partial

### DNN Face Detector
- Model deep learning pre-trained (Caffe/TensorFlow)
- Lebih akurat, bisa deteksi wajah dari berbagai sudut
- Sedikit lebih lambat tapi masih real-time

---

## 🛠️ Praktik

### 1. Face Detection dengan Haar Cascade

```python
"""
sesi07_haar_cascade.py
Deteksi wajah dengan Haar Cascade — metode klasik
"""
import cv2
import imutils

# Load Haar Cascade model (sudah ada di OpenCV)
face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)
eye_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_eye.xml'
)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    # Deteksi wajah
    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.1,    # Faktor skala piramida
        minNeighbors=5,     # Minimum deteksi untuk valid
        minSize=(50, 50)    # Ukuran minimum wajah
    )

    for (x, y, w, h) in faces:
        # Gambar kotak wajah
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.putText(frame, "Face", (x, y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

        # Deteksi mata di dalam area wajah
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = frame[y:y+h, x:x+w]

        eyes = eye_cascade.detectMultiScale(roi_gray, 1.1, 3, minSize=(20, 20))
        for (ex, ey, ew, eh) in eyes:
            cv2.rectangle(roi_color, (ex, ey), (ex + ew, ey + eh),
                          (255, 0, 0), 2)

    cv2.putText(frame, f"Faces: {len(faces)} (Haar)", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("Haar Cascade", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 2. Face Detection dengan DNN

```python
"""
sesi07_dnn_face.py
Deteksi wajah dengan DNN — lebih akurat

Download model:
  - deploy.prototxt: https://raw.githubusercontent.com/opencv/opencv/master/samples/dnn/face_detector/deploy.prototxt
  - res10_300x300_ssd_iter_140000.caffemodel: https://raw.githubusercontent.com/opencv/opencv_3rdparty/dnn_samples_face_detector_20170830/res10_300x300_ssd_iter_140000.caffemodel
"""
import cv2
import numpy as np
import imutils

# Load DNN model
# Jika belum ada, download dulu (lihat petunjuk di atas)
PROTOTXT = "models/deploy.prototxt"
CAFFEMODEL = "models/res10_300x300_ssd_iter_140000.caffemodel"

try:
    net = cv2.dnn.readNetFromCaffe(PROTOTXT, CAFFEMODEL)
    print("DNN model loaded successfully!")
except:
    print("ERROR: Model tidak ditemukan!")
    print("Download dari link di komentar kode, simpan ke folder 'models/'")
    exit()

CONFIDENCE_THRESHOLD = 0.5

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    h, w = frame.shape[:2]

    # Siapkan input blob untuk DNN
    blob = cv2.dnn.blobFromImage(
        cv2.resize(frame, (300, 300)),  # Resize ke 300x300
        1.0,                              # Scale factor
        (300, 300),                       # Size
        (104.0, 177.0, 123.0),           # Mean subtraction
        False, False
    )

    # Forward pass
    net.setInput(blob)
    detections = net.forward()

    face_count = 0
    for i in range(detections.shape[2]):
        confidence = detections[0, 0, i, 2]

        if confidence > CONFIDENCE_THRESHOLD:
            face_count += 1

            # Koordinat bounding box
            box = detections[0, 0, i, 3:7] * np.array([w, h, w, h])
            x1, y1, x2, y2 = box.astype("int")

            # Clamp ke batas frame
            x1, y1 = max(0, x1), max(0, y1)
            x2, y2 = min(w, x2), min(h, y2)

            # Gambar kotak + confidence
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            label = f"Face: {confidence:.2f}"
            cv2.putText(frame, label, (x1, y1 - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    cv2.putText(frame, f"Faces: {face_count} (DNN)", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("DNN Face Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 3. Perbandingan Side-by-Side

```python
"""
sesi07_comparison.py
Bandingkan Haar vs DNN secara side-by-side
"""
import cv2
import numpy as np
import imutils
import time

# Load models
face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)
net = cv2.dnn.readNetFromCaffe("models/deploy.prototxt",
                                "models/res10_300x300_ssd_iter_140000.caffemodel")

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    h, w = frame.shape[:2]

    # === HAAR CASCADE ===
    haar_frame = frame.copy()
    t1 = time.time()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces_haar = face_cascade.detectMultiScale(gray, 1.1, 5, minSize=(50, 50))
    haar_time = (time.time() - t1) * 1000

    for (x, y, fw, fh) in faces_haar:
        cv2.rectangle(haar_frame, (x, y), (x + fw, y + fh), (0, 255, 0), 2)

    cv2.putText(haar_frame, f"HAAR: {len(faces_haar)} faces, {haar_time:.1f}ms",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    # === DNN ===
    dnn_frame = frame.copy()
    t2 = time.time()
    blob = cv2.dnn.blobFromImage(cv2.resize(frame, (300, 300)), 1.0,
                                  (300, 300), (104.0, 177.0, 123.0))
    net.setInput(blob)
    detections = net.forward()
    dnn_time = (time.time() - t2) * 1000

    dnn_count = 0
    for i in range(detections.shape[2]):
        conf = detections[0, 0, i, 2]
        if conf > 0.5:
            dnn_count += 1
            box = detections[0, 0, i, 3:7] * np.array([w, h, w, h])
            x1, y1, x2, y2 = box.astype("int")
            cv2.rectangle(dnn_frame, (x1, y1), (x2, y2), (0, 0, 255), 2)

    cv2.putText(dnn_frame, f"DNN: {dnn_count} faces, {dnn_time:.1f}ms",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

    # Gabung side by side
    combined = np.hstack([haar_frame, dnn_frame])
    cv2.imshow("Haar vs DNN", combined)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 4. Crop & Simpan Wajah

```python
"""
sesi07_save_faces.py
Crop dan simpan wajah terdeteksi ke folder
Berguna untuk membuat dataset face recognition di Sesi 8
"""
import cv2
import imutils
import os
import time

SAVE_DIR = "dataset/faces"
os.makedirs(SAVE_DIR, exist_ok=True)

face_cascade = cv2.CascadeClassifier(
    cv2.data.haarcascades + 'haarcascade_frontalface_default.xml'
)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

person_name = input("Masukkan nama orang: ").strip()
person_dir = os.path.join(SAVE_DIR, person_name)
os.makedirs(person_dir, exist_ok=True)

count = 0
print(f"\nCapturing faces for '{person_name}'")
print("Tekan 'c' untuk capture, 'q' untuk selesai")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    faces = face_cascade.detectMultiScale(gray, 1.1, 5, minSize=(80, 80))

    display = frame.copy()
    for (x, y, w, h) in faces:
        # Padding sedikit agar wajah tidak terlalu tight
        pad = 20
        x1 = max(0, x - pad)
        y1 = max(0, y - pad)
        x2 = min(frame.shape[1], x + w + pad)
        y2 = min(frame.shape[0], y + h + pad)
        cv2.rectangle(display, (x1, y1), (x2, y2), (0, 255, 0), 2)

    cv2.putText(display, f"Captured: {count} | Person: {person_name}",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    cv2.imshow("Capture Faces", display)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('c') and len(faces) > 0:
        for (x, y, w, h) in faces:
            pad = 20
            x1, y1 = max(0, x - pad), max(0, y - pad)
            x2, y2 = min(frame.shape[1], x + w + pad), min(frame.shape[0], y + h + pad)
            face_img = frame[y1:y2, x1:x2]

            filename = f"{person_dir}/{person_name}_{count:03d}.jpg"
            cv2.imwrite(filename, face_img)
            count += 1
            print(f"  Saved: {filename}")

print(f"\nTotal captured: {count} faces for '{person_name}'")
cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Test Haar Cascade** dengan wajah miring 45° — apakah masih terdeteksi?
2. **Coba DNN** dari berbagai sudut (profile, miring) — bandingkan dengan Haar
3. **Capture 20-30 foto wajah** Anda menggunakan script save_faces — akan dipakai di Sesi 8
4. **Buat program** yang menghitung jumlah wajah di frame dan kirim angka ke ESP32 OLED

---

## 📚 Referensi
- [OpenCV Haar Cascade](https://docs.opencv.org/4.x/db/d28/tutorial_cascade_classifier.html)
- [DNN Face Detector](https://github.com/opencv/opencv/tree/master/samples/dnn/face_detector)
- [Face Detection Comparison](https://pyimagesearch.com/2018/02/26/face-detection-with-opencv-and-deep-learning/)
