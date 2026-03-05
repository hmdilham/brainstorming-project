# Sesi 17 — Anomaly Detection: Visual Inspection & Kelistrikan

## 🎯 Tujuan Pembelajaran
- Memahami konsep anomaly detection visual (template matching, SSIM)
- Mendeteksi cacat/perbedaan pada objek dibanding referensi
- Konsep anomaly kelistrikan: deteksi percikan api, panas berlebih, kabel terkelupas
- 🔨 Mini Project: Defect Detector + Electrical Fire Prevention System

---

## 📖 Teori

### Visual Anomaly Detection
```
Referensi (Good)     Test Image      Comparison
┌──────────┐       ┌──────────┐     ┌──────────┐
│  ████    │       │  ██ █    │     │     █    │ ← Anomaly
│  ████    │  vs   │  ████    │  =  │          │
│  ████    │       │  ████    │     │          │
└──────────┘       └──────────┘     └──────────┘
                                    SSIM < threshold → DEFECT!
```

### SSIM (Structural Similarity Index)
- Range: -1 hingga 1 (1 = identik)
- SSIM > 0.9 → produk OK
- SSIM < 0.9 → kemungkinkan cacat

### Anomaly Kelistrikan (Pencegahan Kebakaran)
Kombinasi **Computer Vision** + **Sensor** untuk deteksi dini potensi kebakaran:
```
┌────────────────────────────────────────────────────────┐
│  CV Detection                 │  Sensor Detection      │
│  ─────────────                │  ────────────────      │
│  • Deteksi percikan api       │  • Sensor arus (ACS712)│
│    (warna kuning/oranye)      │  • Sensor suhu (DS18B20│
│  • Deteksi asap (opacity)     │    atau thermistor)    │
│  • Deteksi kabel terkelupas   │  • Sensor gas (MQ-2)   │
│    (warna tembaga exposed)    │                        │
└────────────────────────────────────────────────────────┘
                     │ Salah satu trigger
              ┌──────▼──────┐
              │ ESP32 Relay │ → matikan MCB / power
              │ Buzzer      │ → alarm
              │ OLED        │ → status display
              └─────────────┘
```

---

## 🛠️ Praktik

### 1. Template Matching untuk Visual Inspection

```python
"""
sesi17_template_match.py
Deteksi cacat produk dengan template matching
"""
import cv2
import numpy as np
import imutils

# Capture referensi terlebih dahulu
cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

reference = None
DETECT_ZONE = (180, 140, 280, 280)

print("=== Visual Inspection System ===")
print("Tekan 'r' untuk capture REFERENCE (produk bagus)")
print("Tekan 'c' untuk COMPARE (cek produk baru)")
print("Tekan 'q' untuk keluar")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)

    x, y, w, h = DETECT_ZONE
    cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 255, 255), 2)
    cv2.putText(frame, "INSPECTION AREA", (x, y - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

    if reference is not None:
        cv2.putText(frame, "Reference: SET", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
    else:
        cv2.putText(frame, "Reference: NOT SET", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 255), 2)

    cv2.imshow("Visual Inspection", frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('r'):
        # Capture reference
        roi = frame[y:y+h, x:x+w]
        reference = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
        reference = cv2.GaussianBlur(reference, (7, 7), 0)
        cv2.imwrite("reference.jpg", roi)
        print("✅ Reference captured!")

    elif key == ord('c') and reference is not None:
        # Compare
        roi = frame[y:y+h, x:x+w]
        test = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
        test = cv2.GaussianBlur(test, (7, 7), 0)

        # SSIM comparison
        from skimage.metrics import structural_similarity as ssim
        score, diff = ssim(reference, test, full=True)
        diff = (diff * 255).astype("uint8")

        # Threshold diff untuk highlight anomaly
        _, thresh = cv2.threshold(diff, 200, 255, cv2.THRESH_BINARY_INV)

        # Cari contour anomaly
        contours, _ = cv2.findContours(thresh, cv2.RETR_EXTERNAL,
                                        cv2.CHAIN_APPROX_SIMPLE)

        result = roi.copy()
        defect_count = 0
        for cnt in contours:
            area = cv2.contourArea(cnt)
            if area > 100:
                defect_count += 1
                cx, cy, cw, ch = cv2.boundingRect(cnt)
                cv2.rectangle(result, (cx, cy), (cx+cw, cy+ch), (0, 0, 255), 2)
                cv2.putText(result, "DEFECT", (cx, cy - 5),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 0, 255), 1)

        status = "PASS ✅" if score > 0.85 else "FAIL ❌"
        color = (0, 255, 0) if score > 0.85 else (0, 0, 255)

        cv2.putText(result, f"SSIM: {score:.3f} | {status}", (5, 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)
        cv2.putText(result, f"Defects: {defect_count}", (5, 45),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)

        cv2.imshow("Result", result)
        cv2.imshow("Diff Map", diff)
        cv2.imshow("Threshold", thresh)

        print(f"  SSIM: {score:.3f} | Status: {status} | Defects: {defect_count}")

cap.release()
cv2.destroyAllWindows()
```

> 💡 Install: `pip install scikit-image`

### 2. 🔨 Mini Project: Electrical Fire Prevention (Deteksi Percikan Api)

```python
"""
sesi17_fire_detection.py
🔨 MINI PROJECT: Anomaly Detection Kelistrikan — Pencegah Kebakaran

Deteksi dengan CV:
  - Warna api/percikan (kuning-oranye-merah terang di HSV)
  - Area terang tidak biasa (overexposure)

Aksi:
  - Matikan relay (putus listrik)
  - Buzzer alarm
  - Log event

Bisa digabung dengan sensor arus (ACS712) di ESP32 untuk deteksi overcurrent.
"""
import cv2
import numpy as np
import imutils
import time
import os

# Uncomment untuk Serial ESP32:
# import serial
# ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
# time.sleep(2)

cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

# HSV range untuk percikan api / nyala api
FIRE_RANGES = [
    # Kuning-oranye terang (percikan listrik)
    (np.array([15, 100, 200]), np.array([35, 255, 255])),
    # Oranye-merah (nyala api kecil)
    (np.array([0, 120, 200]), np.array([15, 255, 255])),
    # Putih terang (hot spot)
    (np.array([0, 0, 240]), np.array([179, 30, 255])),
]

# Threshold
FIRE_PIXEL_THRESHOLD = 300   # Minimal pixel api untuk alarm
ALERT_COOLDOWN = 5           # Detik antar alert
LOG_DIR = "fire_logs"
os.makedirs(LOG_DIR, exist_ok=True)

last_alert_time = 0
alert_active = False
system_armed = True

# Background subtractor untuk deteksi perubahan mendadak
bg_sub = cv2.createBackgroundSubtractorMOG2(history=300, varThreshold=30)

# Learning phase
print("⚡ Electrical Fire Prevention System")
print("Learning environment...")
for _ in range(60):
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

    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    hsv_blur = cv2.GaussianBlur(hsv, (5, 5), 0)

    # Deteksi api/percikan
    fire_mask = np.zeros(hsv.shape[:2], dtype=np.uint8)
    for (lower, upper) in FIRE_RANGES:
        fire_mask = cv2.bitwise_or(fire_mask, cv2.inRange(hsv_blur, lower, upper))

    fire_mask = cv2.dilate(fire_mask, None, iterations=2)
    fire_mask = cv2.erode(fire_mask, None, iterations=1)

    # Motion detection (perubahan mendadak)
    fg_mask = bg_sub.apply(frame, learningRate=0.005)
    _, fg_mask = cv2.threshold(fg_mask, 200, 255, cv2.THRESH_BINARY)

    # Gabungkan: fire HANYA di area yang berubah (mengurangi false positive)
    combined_mask = cv2.bitwise_and(fire_mask, fg_mask)

    # Cari area api
    contours, _ = cv2.findContours(combined_mask, cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)

    display = frame.copy()
    fire_detected = False
    total_fire_pixels = 0

    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > 50:
            total_fire_pixels += area
            x, y, w, h = cv2.boundingRect(cnt)
            cv2.rectangle(display, (x, y), (x+w, y+h), (0, 0, 255), 2)
            cv2.putText(display, "FIRE!", (x, y - 5),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)

    if total_fire_pixels > FIRE_PIXEL_THRESHOLD:
        fire_detected = True

    # Alert logic
    now = time.time()
    if fire_detected and system_armed:
        if now - last_alert_time > ALERT_COOLDOWN:
            alert_active = True
            last_alert_time = now

            print(f"🔥 FIRE ALERT! [{timestamp}] Pixels: {total_fire_pixels}")

            # Aksi:
            # 1. Matikan relay (putus listrik)
            # ser.write(b"RELAY_OFF\n")
            # 2. Buzzer alarm
            # ser.write(b"BUZZ_CONTINUOUS\n")

            # 3. Simpan foto
            filename = f"{LOG_DIR}/fire_{int(now)}.jpg"
            cv2.imwrite(filename, frame)
            print(f"   Saved: {filename}")

    elif not fire_detected and now - last_alert_time > 3:
        alert_active = False

    # UI
    if alert_active:
        # Merah berkedip
        if int(now * 3) % 2:
            cv2.rectangle(display, (0, 0),
                          (display.shape[1]-1, display.shape[0]-1), (0, 0, 255), 6)
        cv2.putText(display, "!!! FIRE DETECTED - POWER CUT !!!", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
    else:
        status_color = (0, 255, 0) if system_armed else (100, 100, 100)
        status_text = "MONITORING" if system_armed else "DISARMED"
        cv2.putText(display, f"[{status_text}] {timestamp}", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, status_color, 2)

    cv2.putText(display, f"Fire pixels: {total_fire_pixels}", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

    cv2.imshow("Fire Prevention", display)
    cv2.imshow("Fire Mask", fire_mask)
    cv2.imshow("Combined", combined_mask)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('a'):
        system_armed = not system_armed
        print(f"System {'ARMED' if system_armed else 'DISARMED'}")

cap.release()
cv2.destroyAllWindows()
```

### ESP32 Firmware: Fire Prevention Controller

```cpp
/**
 * sesi17_fire_prevention.ino
 * ESP32 controller untuk sistem pencegah kebakaran
 * Terima perintah dari Python, kontrol relay + buzzer + sensor
 *
 * Opsional sensor:
 * - ACS712 (sensor arus) → deteksi overcurrent
 * - DS18B20 / NTC (sensor suhu) → deteksi overheat
 * - MQ-2 (sensor gas/asap)
 */

#define RELAY_PIN   4     // Relay untuk putus listrik
#define BUZZER_PIN  5
#define LED_RED     2
#define LED_GREEN   15

// Opsional sensor pins
#define ACS712_PIN  34    // Analog sensor arus
#define TEMP_PIN    35    // Analog sensor suhu (NTC)
// #define MQ2_PIN  32    // Opsional gas sensor

bool alarmActive = false;
float CURRENT_THRESHOLD = 10.0;  // Ampere
float TEMP_THRESHOLD = 70.0;     // Celcius

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    // Default: relay ON (listrik mengalir), alarm OFF
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);

    Serial.println("Fire Prevention System Ready");
}

void activateAlarm() {
    alarmActive = true;
    digitalWrite(RELAY_PIN, LOW);    // MATIKAN listrik!
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    Serial.println(">> ALARM ACTIVE - POWER CUT!");
}

void deactivateAlarm() {
    alarmActive = false;
    digitalWrite(RELAY_PIN, HIGH);   // Nyalakan kembali
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
    Serial.println(">> ALARM DEACTIVATED");
}

float readCurrent() {
    // ACS712 5A: sensitivity 185mV/A, offset 2.5V
    int raw = analogRead(ACS712_PIN);
    float voltage = (raw / 4095.0) * 3.3;
    float current = (voltage - 1.65) / 0.185;
    return abs(current);
}

float readTemperature() {
    // Simple NTC thermistor reading (approximate)
    int raw = analogRead(TEMP_PIN);
    float voltage = (raw / 4095.0) * 3.3;
    // Simplified calculation
    float temperature = voltage * 100;  // Adjust based on your NTC
    return temperature;
}

void loop() {
    // Cek perintah dari Python (CV detection)
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();

        if (cmd == "RELAY_OFF" || cmd == "FIRE_ALERT") {
            activateAlarm();
        }
        else if (cmd == "RELAY_ON" || cmd == "RESET") {
            deactivateAlarm();
        }
        else if (cmd == "BUZZ_CONTINUOUS") {
            digitalWrite(BUZZER_PIN, HIGH);
        }
        else if (cmd == "BUZZ_STOP") {
            digitalWrite(BUZZER_PIN, LOW);
        }
    }

    // === Sensor-based detection (backup dari CV) ===

    // Cek overcurrent
    float current = readCurrent();
    if (current > CURRENT_THRESHOLD && !alarmActive) {
        Serial.printf("WARNING: Overcurrent! %.1f A\n", current);
        activateAlarm();
    }

    // Cek suhu
    float temp = readTemperature();
    if (temp > TEMP_THRESHOLD && !alarmActive) {
        Serial.printf("WARNING: Overtemp! %.1f C\n", temp);
        activateAlarm();
    }

    // Buzzer pattern saat alarm
    if (alarmActive) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
    }

    // Kirim data sensor ke Python (opsional monitoring)
    static unsigned long lastReport = 0;
    if (millis() - lastReport > 2000) {
        Serial.printf("SENSOR:I=%.1f,T=%.0f\n", current, temp);
        lastReport = millis();
    }
}
```

---

## 📝 Latihan Mandiri

1. **Setup SSIM inspector** dengan objek sederhana (bolpoin, buku, dll)
2. **Test fire detection** dengan gambar api dari layar HP (hati-hati ambiguity cahaya!)
3. **Tambahkan sensor ACS712** untuk deteksi arus berlebih
4. **Buat dashboard** yang menampilkan status sensor + camera feed

---

## 📚 Referensi
- [SSIM - scikit-image](https://scikit-image.org/docs/dev/api/skimage.metrics.html#skimage.metrics.structural_similarity)
- [Fire Detection OpenCV](https://pyimagesearch.com/2019/11/18/fire-and-smoke-detection-with-keras-and-deep-learning/)
- [ACS712 Current Sensor](https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/)
