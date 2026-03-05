# Sesi 8 — Face Recognition (Pengenalan Wajah)

## 🎯 Tujuan Pembelajaran
- Memahami konsep face embedding dan vector comparison
- Mengenroll wajah baru ke database
- Mengenali wajah secara real-time
- 🔨 Mini Project: Face Unlock → Servo membuka kunci

---

## 📖 Teori

### Face Recognition Pipeline
```
Capture Frame → Detect Face → Encode (128-d vector) → Compare → Match/Unknown
                                        ↕
                              Database (known faces)
```

### Face Embedding
Wajah direpresentasikan sebagai vektor 128 dimensi. Wajah yang mirip memiliki jarak vektor yang kecil.
```
Ilham  → [0.12, -0.34, 0.56, ..., 0.78]  (128 angka)
Andi   → [0.87, 0.21, -0.45, ..., 0.11]  (128 angka)

distance(Ilham, Ilham') = 0.3  ← Match! (< 0.6)
distance(Ilham, Andi)   = 1.1  ← Beda orang (> 0.6)
```

---

## 🛠️ Praktik

### 0. Install Library

```bash
pip install face_recognition dlib
# Jika dlib gagal install di Linux:
# sudo apt install cmake libopenblas-dev liblapack-dev
# pip install dlib face_recognition
```

### 1. Enroll Wajah (Buat Database)

```python
"""
sesi08_enroll_faces.py
Mendaftarkan wajah ke database encoding
Gunakan foto dari Sesi 7 atau ambil langsung dari kamera
"""
import face_recognition
import cv2
import os
import pickle
import imutils

DATASET_DIR = "dataset/faces"   # Folder dari Sesi 7
ENCODING_FILE = "face_encodings.pkl"

known_encodings = []
known_names = []

print("=== Encoding faces from dataset ===\n")

for person_name in os.listdir(DATASET_DIR):
    person_dir = os.path.join(DATASET_DIR, person_name)
    if not os.path.isdir(person_dir):
        continue

    print(f"Processing: {person_name}")
    count = 0

    for img_name in os.listdir(person_dir):
        img_path = os.path.join(person_dir, img_name)
        img = cv2.imread(img_path)

        if img is None:
            continue

        # Convert BGR → RGB (face_recognition butuh RGB)
        rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

        # Detect dan encode wajah
        boxes = face_recognition.face_locations(rgb, model="hog")
        encodings = face_recognition.face_encodings(rgb, boxes)

        for encoding in encodings:
            known_encodings.append(encoding)
            known_names.append(person_name)
            count += 1

    print(f"  → {count} faces encoded")

# Simpan database
data = {"encodings": known_encodings, "names": known_names}
with open(ENCODING_FILE, "wb") as f:
    pickle.dump(data, f)

print(f"\n✅ Database saved: {ENCODING_FILE}")
print(f"   Total: {len(known_encodings)} encodings for {len(set(known_names))} people")
```

### 2. Enroll Langsung dari Kamera

```python
"""
sesi08_enroll_live.py
Enroll wajah langsung dari webcam / ESP32-CAM
"""
import face_recognition
import cv2
import pickle
import imutils
import os

ENCODING_FILE = "face_encodings.pkl"

# Load existing database jika ada
if os.path.exists(ENCODING_FILE):
    with open(ENCODING_FILE, "rb") as f:
        data = pickle.load(f)
    known_encodings = data["encodings"]
    known_names = data["names"]
    print(f"Loaded {len(known_encodings)} existing encodings")
else:
    known_encodings = []
    known_names = []

name = input("Nama orang yang akan didaftarkan: ").strip()
print(f"\nArahkan wajah ke kamera, tekan 'c' untuk capture (target: 5-10 foto)")
print("Tekan 'q' setelah selesai\n")

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

captured = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Detect face
    boxes = face_recognition.face_locations(rgb, model="hog")

    display = frame.copy()
    for (top, right, bottom, left) in boxes:
        cv2.rectangle(display, (left, top), (right, bottom), (0, 255, 0), 2)

    cv2.putText(display, f"Enrolling: {name} | Captured: {captured}",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    cv2.imshow("Enroll", display)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('c') and len(boxes) > 0:
        encodings = face_recognition.face_encodings(rgb, boxes)
        for enc in encodings:
            known_encodings.append(enc)
            known_names.append(name)
            captured += 1
            print(f"  Captured #{captured}")

# Save updated database
data = {"encodings": known_encodings, "names": known_names}
with open(ENCODING_FILE, "wb") as f:
    pickle.dump(data, f)

print(f"\n✅ Enrolled {captured} faces for '{name}'")
cap.release()
cv2.destroyAllWindows()
```

### 3. Face Recognition Real-time

```python
"""
sesi08_face_recognition.py
Pengenalan wajah real-time dari kamera
"""
import face_recognition
import cv2
import pickle
import imutils

ENCODING_FILE = "face_encodings.pkl"
TOLERANCE = 0.5   # Semakin kecil = semakin strict

# Load database
with open(ENCODING_FILE, "rb") as f:
    data = pickle.load(f)

known_encodings = data["encodings"]
known_names = data["names"]
print(f"Loaded {len(known_encodings)} encodings for: {set(known_names)}")

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# Proses setiap N frame untuk performa
PROCESS_EVERY = 2
frame_count = 0
face_results = []

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame_count += 1

    # Proses setiap N frame
    if frame_count % PROCESS_EVERY == 0:
        # Resize kecil untuk kecepatan
        small = cv2.resize(frame, (0, 0), fx=0.5, fy=0.5)
        rgb_small = cv2.cvtColor(small, cv2.COLOR_BGR2RGB)

        # Detect dan encode
        boxes = face_recognition.face_locations(rgb_small, model="hog")
        encodings = face_recognition.face_encodings(rgb_small, boxes)

        face_results = []
        for (encoding, (top, right, bottom, left)) in zip(encodings, boxes):
            # Scale back koordinat
            top *= 2; right *= 2; bottom *= 2; left *= 2

            # Compare
            matches = face_recognition.compare_faces(known_encodings, encoding,
                                                      tolerance=TOLERANCE)
            distances = face_recognition.face_distance(known_encodings, encoding)

            name = "Unknown"
            confidence = 0.0

            if True in matches:
                # Ambil yang paling mirip
                best_idx = distances.argmin()
                if matches[best_idx]:
                    name = known_names[best_idx]
                    confidence = 1.0 - distances[best_idx]

            face_results.append((name, confidence, (top, right, bottom, left)))

    # Gambar hasil
    display = frame.copy()
    for (name, conf, (top, right, bottom, left)) in face_results:
        color = (0, 255, 0) if name != "Unknown" else (0, 0, 255)

        cv2.rectangle(display, (left, top), (right, bottom), color, 2)

        label = f"{name} ({conf:.0%})" if name != "Unknown" else "Unknown"
        cv2.rectangle(display, (left, bottom), (right, bottom + 25), color, -1)
        cv2.putText(display, label, (left + 5, bottom + 18),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

    cv2.imshow("Face Recognition", display)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### 4. 🔨 Mini Project: Face Unlock Door

```python
"""
sesi08_face_unlock.py
🔨 MINI PROJECT: Face Unlock — buka kunci (servo) jika wajah dikenali

Pipeline:
  Kamera → face_recognition → match? → ESP32 → Servo → buka kunci
"""
import face_recognition
import cv2
import pickle
import imutils
import time

# Uncomment untuk komunikasi ESP32:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

ENCODING_FILE = "face_encodings.pkl"
AUTHORIZED_NAMES = ["ilham", "admin"]  # Daftar yang boleh buka kunci
TOLERANCE = 0.5
UNLOCK_DURATION = 5  # Detik kunci terbuka

with open(ENCODING_FILE, "rb") as f:
    data = pickle.load(f)

known_encodings = data["encodings"]
known_names = data["names"]

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

lock_state = "LOCKED"
unlock_time = 0

print("Face Unlock System Active!")
print(f"Authorized: {AUTHORIZED_NAMES}")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    small = cv2.resize(frame, (0, 0), fx=0.5, fy=0.5)
    rgb_small = cv2.cvtColor(small, cv2.COLOR_BGR2RGB)

    # Detect
    boxes = face_recognition.face_locations(rgb_small, model="hog")
    encodings = face_recognition.face_encodings(rgb_small, boxes)

    display = frame.copy()
    detected_name = None

    for (enc, (top, right, bottom, left)) in zip(encodings, boxes):
        top *= 2; right *= 2; bottom *= 2; left *= 2

        matches = face_recognition.compare_faces(known_encodings, enc,
                                                  tolerance=TOLERANCE)
        distances = face_recognition.face_distance(known_encodings, enc)

        name = "Unknown"
        if True in matches:
            best_idx = distances.argmin()
            if matches[best_idx]:
                name = known_names[best_idx]

        color = (0, 255, 0) if name in AUTHORIZED_NAMES else (0, 0, 255)
        cv2.rectangle(display, (left, top), (right, bottom), color, 2)
        cv2.putText(display, name, (left, top - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)

        if name in AUTHORIZED_NAMES:
            detected_name = name

    # Logic unlock
    current_time = time.time()
    if detected_name and lock_state == "LOCKED":
        lock_state = "UNLOCKED"
        unlock_time = current_time
        print(f"🔓 UNLOCKED by: {detected_name}")
        # ser.write(b"UNLOCK\n")   # Kirim ke ESP32 → servo

    elif lock_state == "UNLOCKED" and current_time - unlock_time > UNLOCK_DURATION:
        lock_state = "LOCKED"
        print("🔒 AUTO-LOCKED")
        # ser.write(b"LOCK\n")

    # Status overlay
    if lock_state == "UNLOCKED":
        cv2.putText(display, "🔓 UNLOCKED", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        remaining = UNLOCK_DURATION - (current_time - unlock_time)
        cv2.putText(display, f"Locks in: {remaining:.0f}s", (10, 65),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
    else:
        cv2.putText(display, "🔒 LOCKED", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)

    cv2.imshow("Face Unlock", display)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

### ESP32 Firmware untuk Servo Lock

```cpp
/**
 * sesi08_servo_lock.ino
 * ESP32 kontrolkan servo untuk buka/kunci pintu
 */
#include <ESP32Servo.h>

#define SERVO_PIN  13
#define LED_GREEN  2
#define LED_RED    4

Servo lockServo;

void setup() {
    Serial.begin(115200);
    lockServo.attach(SERVO_PIN);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_RED, OUTPUT);

    // Posisi awal: terkunci
    lockServo.write(0);
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);

    Serial.println("Servo Lock Ready");
}

void loop() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "UNLOCK") {
            lockServo.write(90);    // Buka kunci
            digitalWrite(LED_GREEN, HIGH);
            digitalWrite(LED_RED, LOW);
            Serial.println(">> UNLOCKED");
        }
        else if (cmd == "LOCK") {
            lockServo.write(0);     // Kunci
            digitalWrite(LED_GREEN, LOW);
            digitalWrite(LED_RED, HIGH);
            Serial.println(">> LOCKED");
        }
    }
}
```

---

## 📝 Latihan Mandiri

1. **Enroll** minimal 3 orang berbeda dan test recognition accuracy
2. **Coba variasi** toleransi (0.4, 0.5, 0.6) dan amati dampaknya
3. **Tambahkan logging** — simpan log siapa yang buka kunci, jam berapa
4. **Challenge**: Tambahkan anti-spoofing sederhana (blink detection)

---

## 📚 Referensi
- [face_recognition Library](https://github.com/ageitgey/face_recognition)
- [Face Recognition Tutorial](https://pyimagesearch.com/2018/06/18/face-recognition-with-opencv-python-and-deep-learning/)
- [dlib Documentation](http://dlib.net/)
