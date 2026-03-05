# Sesi 18 — Anomaly Detection: Behavioral (Pose Estimation)

## 🎯 Tujuan Pembelajaran
- Menggunakan MediaPipe Pose untuk deteksi 33 pose landmarks
- Mengklasifikasi pose (berdiri, duduk, jatuh)
- 🔨 Mini Project: Fall Detection Alert → buzzer + notifikasi ESP32

---

## 📖 Teori

### MediaPipe Pose
MediaPipe Pose mendeteksi **33 landmark** tubuh manusia secara real-time:
```
        (0) Nose
       /    \
   (11)L     R(12)    ← Shoulders
   (13)L     R(14)    ← Elbows
   (15)L     R(16)    ← Wrists
      |       |
   (23)L     R(24)    ← Hips
   (25)L     R(26)    ← Knees
   (27)L     R(28)    ← Ankles
```

### Fall Detection Logic
Orang jatuh bisa dideteksi dari:
1. **Rasio bounding box**: Jatuh = width > height (terlentang)
2. **Posisi hip**: Hip mendekati level ankle = jatuh/jongkok
3. **Kecepatan perubahan posisi**: Perubahan mendadak = jatuh

---

## 🛠️ Praktik

### 1. Pose Detection Dasar

```python
"""
sesi18_pose_detection.py
Deteksi pose tubuh menggunakan MediaPipe Pose
"""
import cv2
import mediapipe as mp
import imutils

mp_pose = mp.solutions.pose
mp_draw = mp.solutions.drawing_utils
mp_styles = mp.solutions.drawing_styles

pose = mp_pose.Pose(
    static_image_mode=False,
    model_complexity=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = pose.process(rgb)

    if results.pose_landmarks:
        mp_draw.draw_landmarks(
            frame, results.pose_landmarks, mp_pose.POSE_CONNECTIONS,
            mp_styles.get_default_pose_landmarks_style()
        )

        # Tampilkan beberapa landmark penting
        h, w, _ = frame.shape
        landmarks = results.pose_landmarks.landmark

        # Nose, shoulders, hips
        nose = landmarks[0]
        l_shoulder = landmarks[11]
        r_shoulder = landmarks[12]
        l_hip = landmarks[23]
        r_hip = landmarks[24]

        info = [
            f"Nose: ({nose.x:.2f}, {nose.y:.2f})",
            f"L.Shoulder: ({l_shoulder.x:.2f}, {l_shoulder.y:.2f})",
            f"L.Hip: ({l_hip.x:.2f}, {l_hip.y:.2f})",
        ]
        for i, text in enumerate(info):
            cv2.putText(frame, text, (10, 30 + i * 25),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

    cv2.imshow("Pose Detection", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

pose.close()
cap.release()
cv2.destroyAllWindows()
```

### 2. Pose Classification (Berdiri/Duduk/Jatuh)

```python
"""
sesi18_pose_classify.py
Klasifikasi pose: Standing, Sitting, Fallen
"""
import cv2
import mediapipe as mp
import imutils
import math

mp_pose = mp.solutions.pose
mp_draw = mp.solutions.drawing_utils

pose = mp_pose.Pose(min_detection_confidence=0.5)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")


def classify_pose(landmarks, frame_h, frame_w):
    """Klasifikasi pose berdasarkan landmark."""
    # Key landmarks
    nose = landmarks[0]
    l_shoulder = landmarks[11]
    r_shoulder = landmarks[12]
    l_hip = landmarks[23]
    r_hip = landmarks[24]
    l_ankle = landmarks[27]
    r_ankle = landmarks[28]

    # Hitung mid-point shoulder dan hip
    mid_shoulder_y = (l_shoulder.y + r_shoulder.y) / 2
    mid_hip_y = (l_hip.y + r_hip.y) / 2
    mid_ankle_y = (l_ankle.y + r_ankle.y) / 2

    mid_shoulder_x = (l_shoulder.x + r_shoulder.x) / 2
    mid_hip_x = (l_hip.x + r_hip.x) / 2

    # Bounding box body
    body_width = abs(l_shoulder.x - r_shoulder.x) * frame_w
    body_height = abs(nose.y - mid_ankle_y) * frame_h

    # Rasio w/h
    if body_height > 0:
        aspect_ratio = body_width / body_height
    else:
        aspect_ratio = 1.0

    # Sudut torso (vertical = berdiri, horizontal = jatuh)
    torso_angle = math.degrees(math.atan2(
        mid_hip_y - mid_shoulder_y,
        mid_hip_x - mid_shoulder_x
    ))

    # Classification logic
    if aspect_ratio > 0.8:
        return "FALLEN", (0, 0, 255), aspect_ratio, torso_angle
    elif mid_hip_y > 0.65 and mid_shoulder_y > 0.45:
        return "SITTING", (0, 255, 255), aspect_ratio, torso_angle
    else:
        return "STANDING", (0, 255, 0), aspect_ratio, torso_angle


while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    results = pose.process(rgb)

    if results.pose_landmarks:
        mp_draw.draw_landmarks(frame, results.pose_landmarks,
                                mp_pose.POSE_CONNECTIONS)

        lm = results.pose_landmarks.landmark
        pose_name, color, ratio, angle = classify_pose(lm, h, w)

        cv2.putText(frame, f"Pose: {pose_name}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2)
        cv2.putText(frame, f"Ratio: {ratio:.2f} | Angle: {angle:.0f}",
                    (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (200, 200, 200), 1)

    cv2.imshow("Pose Classification", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

pose.close()
cap.release()
cv2.destroyAllWindows()
```

### 3. 🔨 Mini Project: Fall Detection Alert

```python
"""
sesi18_fall_detection.py
🔨 MINI PROJECT: Deteksi orang jatuh → alert ke ESP32

Logic:
  - Berdiri: aspect_ratio < 0.6 → OK
  - Jatuh:   aspect_ratio > 0.8 selama >1 detik → ALERT!
"""
import cv2
import mediapipe as mp
import imutils
import time
import math

# Uncomment untuk ESP32:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

mp_pose = mp.solutions.pose
mp_draw = mp.solutions.drawing_utils

pose = mp_pose.Pose(min_detection_confidence=0.5)
cap = cv2.VideoCapture(0)

fall_start_time = None
FALL_DURATION = 1.5  # Harus jatuh >1.5 detik untuk trigger
alert_active = False
alert_cooldown = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)
    h, w, _ = frame.shape
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = pose.process(rgb)

    now = time.time()
    is_fallen = False

    if results.pose_landmarks:
        mp_draw.draw_landmarks(frame, results.pose_landmarks,
                                mp_pose.POSE_CONNECTIONS)

        lm = results.pose_landmarks.landmark
        l_sh, r_sh = lm[11], lm[12]
        l_hi, r_hi = lm[23], lm[24]
        nose = lm[0]
        l_ankle, r_ankle = lm[27], lm[28]

        body_w = abs(l_sh.x - r_sh.x) * w
        body_h = abs(nose.y - (l_ankle.y + r_ankle.y) / 2) * h

        if body_h > 0:
            ratio = body_w / body_h
        else:
            ratio = 0

        if ratio > 0.8:
            is_fallen = True

    # Fall timer logic
    if is_fallen:
        if fall_start_time is None:
            fall_start_time = now
        elif now - fall_start_time > FALL_DURATION and not alert_active:
            alert_active = True
            alert_cooldown = now
            print("⚠️ FALL DETECTED! Sending alert...")
            # ser.write(b"FALL_ALERT\n")
    else:
        fall_start_time = None
        if alert_active and now - alert_cooldown > 5:
            alert_active = False
            # ser.write(b"ALERT_CLEAR\n")

    # UI
    if alert_active:
        if int(now * 3) % 2:
            cv2.rectangle(frame, (0, 0), (w-1, h-1), (0, 0, 255), 5)
        cv2.putText(frame, "!!! FALL DETECTED !!!", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2)
        cv2.putText(frame, "Alert sent to ESP32", (10, 65),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)
    elif is_fallen:
        elapsed = now - fall_start_time if fall_start_time else 0
        cv2.putText(frame, f"FALLING... ({elapsed:.1f}s)", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 128, 255), 2)
    else:
        cv2.putText(frame, "Status: OK", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 0), 2)

    cv2.imshow("Fall Detection", frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

pose.close()
cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Tambah pose** "Hands Up" (kedua tangan di atas kepala)
2. **Buat drowsiness detector** menggunakan face landmark (Eye Aspect Ratio)
3. **Coba di ESP32-CAM** stream dan ukur delay/latency
4. **Challenge**: Deteksi workout (push-up counter menggunakan pose landmarks)

---

## 📚 Referensi
- [MediaPipe Pose](https://google.github.io/mediapipe/solutions/pose.html)
- [Fall Detection Research](https://arxiv.org/search/?searchtype=all&query=fall+detection+pose)
