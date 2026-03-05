# Sesi 26 — Edge Impulse: Image Classification On-Device

## 🎯 Tujuan Pembelajaran
- Memahami transfer learning untuk image classification
- Mengumpulkan gambar dari ESP32-CAM untuk training
- Training image classifier dengan MobileNet di Edge Impulse
- Quantization INT8 dan deploy ke ESP32
- 🔨 Mini Project: Smart Trash Bin — klasifikasi sampah → servo otomatis

---

## 📖 Teori

### Image Classification vs Object Detection

```
IMAGE CLASSIFICATION:                  OBJECT DETECTION:
"ADA APA di gambar ini?"               "DI MANA benda-benda di gambar?"

┌──────────────┐                       ┌──────────────┐
│              │                       │  ┌──┐  ┌──┐ │
│   🍎         │  → "Apel" (95%)       │  │🍎│  │🍌│ │  → Apel (x,y,w,h)
│              │                       │  └──┘  └──┘ │    Pisang (x,y,w,h)
└──────────────┘                       └──────────────┘

Mana yang bisa jalan di ESP32?
✅ Classification → model kecil (~100KB), input kecil (96x96)
❌ Detection → model besar (>1MB), butuh banyak RAM
```

### Transfer Learning

Kita TIDAK melatih model dari nol. Kita mengambil model yang sudah pintar (MobileNet, sudah belajar jutaan gambar), lalu **finetune** hanya bagian akhirnya untuk tugas kita.

```
Analogi:
  🎓 Belajar dari nol:
     Bayi → belajar bentuk → belajar warna → belajar objek → bisa kenali apel
     (Butuh jutaan gambar, berminggu-minggu training)

  📚 Transfer Learning:
     Sudah lulus SMA (MobileNet) → belajar mata kuliah baru (klasifikasi sampah)
     (Hanya butuh 100-200 gambar, beberapa menit training)
```

### MobileNet

**MobileNet** = arsitektur CNN yang dirancang khusus untuk perangkat mobile/edge. Ringan tapi tetap akurat.

```
Ukuran MobileNetV2:
  ┌──────────────────────────────┬──────────┐
  │ Versi                        │ Size     │
  ├──────────────────────────────┼──────────┤
  │ MobileNetV2 Full (Float32)   │ ~14 MB   │  ← terlalu besar
  │ MobileNetV2 α=0.35 (Float32)│ ~1.6 MB  │  ← masih besar
  │ MobileNetV2 α=0.1 (INT8)    │ ~100 KB  │  ← ✅ bisa di ESP32!
  │ MobileNetV2 α=0.05 (INT8)   │ ~50 KB   │  ← ✅ sangat kecil
  └──────────────────────────────┴──────────┘

  α (alpha) = width multiplier (mengurangi jumlah channel)
  Semakin kecil α → model semakin kecil, tapi akurasi turun
```

### Quantization (Kuantisasi)

Mengubah angka di model dari **float32** (4 bytes) menjadi **int8** (1 byte).

```
FLOAT32:                    INT8:
  3.14159265...             3
  -0.00782151...           -1
  127.553200...            127

  1 angka = 4 bytes         1 angka = 1 byte
  Model 1MB                 Model 250KB → ✅ 4x lebih kecil!

  Akurasi: 95.2%            Akurasi: 93.8% → hanya turun sedikit
```

---

## 🛠️ Praktik

### 1. Collect Gambar dari ESP32-CAM

```python
"""
sesi26_collect_images.py
Kumpulkan gambar dari ESP32-CAM untuk training

Jalankan ini di PC, tekan 'c' untuk capture setiap gambar.
Gambar akan disimpan ke folder per kelas.
"""
import cv2
import os
import time

# === KONFIGURASI ===
CLASSES = ["organik", "anorganik", "kertas"]  # Sesuaikan
IMAGES_DIR = "dataset_images"
IMG_SIZE = 96   # Edge Impulse image input size

# Kamera
cap = cv2.VideoCapture(0)
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")

current_class = 0
counts = {c: 0 for c in CLASSES}

# Buat folder
for cls in CLASSES:
    os.makedirs(f"{IMAGES_DIR}/{cls}", exist_ok=True)
    counts[cls] = len(os.listdir(f"{IMAGES_DIR}/{cls}"))

print("=== Image Dataset Collector ===")
print(f"Kelas: {CLASSES}")
print(f"Target: 50-100 gambar per kelas\n")
print("Controls:")
print("  'c' = Capture gambar")
print("  'n' = Next class")
print("  'p' = Previous class")
print("  'q' = Quit\n")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    display = frame.copy()
    cls_name = CLASSES[current_class]

    # Preview area (crop center, resize ke 96x96)
    h, w = frame.shape[:2]
    crop_size = min(h, w) - 40
    y1 = (h - crop_size) // 2
    x1 = (w - crop_size) // 2
    cv2.rectangle(display, (x1, y1), (x1 + crop_size, y1 + crop_size),
                  (0, 255, 0), 2)

    # Info
    cv2.putText(display, f"Class: {cls_name.upper()} ({counts[cls_name]} imgs)",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
    cv2.putText(display, "C=capture N=next P=prev Q=quit", (10, h - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (200, 200, 200), 1)

    # Preview 96x96
    crop = frame[y1:y1+crop_size, x1:x1+crop_size]
    preview = cv2.resize(crop, (IMG_SIZE, IMG_SIZE))
    preview_big = cv2.resize(preview, (150, 150), interpolation=cv2.INTER_NEAREST)
    display[10:160, w-160:w-10] = preview_big

    cv2.imshow("Collector", display)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('c'):
        # Capture & save
        filename = f"{IMAGES_DIR}/{cls_name}/{cls_name}_{counts[cls_name]:04d}.jpg"
        cv2.imwrite(filename, preview)
        counts[cls_name] += 1
        print(f"  ✅ Saved: {filename} ({counts[cls_name]} total)")
    elif key == ord('n'):
        current_class = (current_class + 1) % len(CLASSES)
        print(f"  → Switched to: {CLASSES[current_class]}")
    elif key == ord('p'):
        current_class = (current_class - 1) % len(CLASSES)
        print(f"  → Switched to: {CLASSES[current_class]}")

# Summary
print("\n=== Summary ===")
for cls, count in counts.items():
    print(f"  {cls}: {count} images")

print(f"\nUpload ke Edge Impulse:")
for cls in CLASSES:
    print(f"  edge-impulse-uploader --category training --label {cls} {IMAGES_DIR}/{cls}/*.jpg")

cap.release()
cv2.destroyAllWindows()
```

### 2. Edge Impulse: Design Image Impulse

```
DI EDGE IMPULSE STUDIO:

1. UPLOAD DATA:
   - Klik "Data acquisition" → "Upload data"
   - Upload folder per kelas
   - ATAU gunakan CLI:
     edge-impulse-uploader --category training --label organik dataset_images/organik/*.jpg

2. CREATE IMPULSE:
   ┌───────────────┐     ┌───────────────┐     ┌───────────────┐
   │ Image Data    │────►│ Image         │────►│ Transfer      │
   │               │     │ (resize to    │     │ Learning      │
   │ 96x96 px      │     │  96x96, RGB)  │     │ (MobileNetV2) │
   │ 3 channels    │     │               │     │               │
   └───────────────┘     └───────────────┘     └───────────────┘

   - Image width: 96
   - Image height: 96
   - Resize mode: "Fit shortest axis" (squash)
   - Color depth: RGB

3. IMAGE PROCESSING:
   - Klik "Image" di sidebar
   - Color depth: RGB
   - Klik "Save parameters"
   - Klik "Generate features"

   Feature Explorer harus menunjukkan cluster terpisah per kelas:
   ● organik    (hijau)    → cluster di area tertentu
   ▲ anorganik  (merah)   → cluster di area lain
   ■ kertas     (biru)    → cluster terpisah lagi

4. TRANSFER LEARNING TRAINING:
   - Klik "Transfer learning" di sidebar
   - Settings:
     - Number of training cycles: 50
     - Learning rate: 0.001
     - Data augmentation: ✅ (flip, rotate, crop sedikit)
     - Model: MobileNetV2 96x96 0.1 (paling kecil)
   - Klik "Start training"

   HASIL YANG DIHARAPKAN:
   - Accuracy: 85-95%
   - Model size: < 100 KB (INT8)
   - Inference time: < 200ms di ESP32

5. DEPLOY:
   - Klik "Deployment"
   - Pilih "Arduino library"
   - ✅ Enable EON Compiler
   - ✅ Quantized INT8
   - Klik "Build" → download .zip
```

### 3. 🔨 Mini Project: Smart Trash Bin

```cpp
/**
 * sesi26_smart_trash_bin.ino
 * 🔨 MINI PROJECT: Klasifikasi sampah → servo buka tutup otomatis
 *
 * ESP32-CAM ambil gambar → classify → servo buka bin yang sesuai
 *
 * WIRING:
 *   Servo Organik → GPIO 12
 *   Servo Anorganik → GPIO 13
 *   LED Indicator → GPIO 4 (flash LED built-in ESP32-CAM)
 *
 * SEBELUM UPLOAD:
 * 1. Train model di Edge Impulse (image classification)
 * 2. Deploy sebagai Arduino library
 * 3. Ganti include dengan nama project Anda
 */

// GANTI DENGAN LIBRARY ANDA!
#include <YOUR_PROJECT_NAME_inferencing.h>
#include "esp_camera.h"
#include <ESP32Servo.h>

// === ESP32-CAM Pin Definitions (AI-Thinker) ===
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Servo & LED
#define SERVO_ORGANIK   12
#define SERVO_ANORGANIK 13
#define LED_FLASH        4

Servo servoOrganik;
Servo servoAnorganik;

// Image buffer
#define IMG_WIDTH   96
#define IMG_HEIGHT  96

void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_96X96;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        while (1);
    }
}

void openBin(const char* binType) {
    Serial.printf("🗑️ Opening %s bin...\n", binType);

    if (strcmp(binType, "organik") == 0) {
        servoOrganik.write(90);   // Open
        delay(3000);              // Buka 3 detik
        servoOrganik.write(0);    // Close
    }
    else if (strcmp(binType, "anorganik") == 0 ||
             strcmp(binType, "kertas") == 0) {
        servoAnorganik.write(90);
        delay(3000);
        servoAnorganik.write(0);
    }
}

void setup() {
    Serial.begin(115200);

    // Setup servo
    servoOrganik.attach(SERVO_ORGANIK);
    servoAnorganik.attach(SERVO_ANORGANIK);
    servoOrganik.write(0);
    servoAnorganik.write(0);

    // LED
    pinMode(LED_FLASH, OUTPUT);
    digitalWrite(LED_FLASH, LOW);

    // Camera
    setupCamera();

    Serial.println("================================");
    Serial.println("  🗑️ Smart Trash Bin Ready!");
    Serial.println("  Letakkan sampah di depan kamera");
    Serial.println("================================");
}

void loop() {
    // Ambil foto
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        delay(1000);
        return;
    }

    // Flash LED saat capture
    digitalWrite(LED_FLASH, HIGH);
    delay(50);
    digitalWrite(LED_FLASH, LOW);

    // Siapkan data untuk model
    // Convert RGB565 ke float array yang dibutuhkan model
    signal_t signal;
    // ... (Edge Impulse library handle konversi ini)

    ei_impulse_result_t result;
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err == EI_IMPULSE_OK) {
        // Cari prediksi tertinggi
        float maxVal = 0;
        int maxIdx = 0;
        for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
            Serial.printf("  %s: %.2f  ",
                           ei_classifier_inferencing_categories[i],
                           result.classification[i].value);
            if (result.classification[i].value > maxVal) {
                maxVal = result.classification[i].value;
                maxIdx = i;
            }
        }
        Serial.println();

        const char* label = ei_classifier_inferencing_categories[maxIdx];

        if (maxVal > 0.75) {
            Serial.printf("→ %s (%.0f%%) — Opening bin!\n", label, maxVal * 100);
            openBin(label);
        } else {
            Serial.printf("→ Not sure (%.0f%%) — ignoring\n", maxVal * 100);
        }
    }

    esp_camera_fb_return(fb);

    delay(2000);  // Classify setiap 2 detik
}
```

### 4. [Alternatif] Python Simulator (Tanpa ESP32-CAM)

```python
"""
sesi26_classifier_simulator.py
Simulasi image classification menggunakan webcam
(Untuk testing sebelum deploy ke ESP32)

Menggunakan model TFLite yang di-export dari Edge Impulse
"""
import cv2
import numpy as np
import imutils

# Load TFLite model (download dari Edge Impulse → Deployment → TFLite)
try:
    import tflite_runtime.interpreter as tflite
except ImportError:
    import tensorflow.lite as tflite

MODEL_PATH = "model_quantized_int8.tflite"
LABELS = ["organik", "anorganik", "kertas"]
IMG_SIZE = 96

# Load model
try:
    interpreter = tflite.Interpreter(model_path=MODEL_PATH)
    interpreter.allocate_tensors()
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    print(f"Model loaded: {MODEL_PATH}")
    print(f"Input shape: {input_details[0]['shape']}")
    print(f"Input type: {input_details[0]['dtype']}")
    model_loaded = True
except:
    print(f"⚠️ Model file '{MODEL_PATH}' not found.")
    print("  Download dari Edge Impulse → Deployment → TFLite (int8)")
    print("  Running in DEMO MODE (random predictions)")
    model_loaded = False

cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    display = frame.copy()
    h, w = frame.shape[:2]

    # Crop center square
    crop_size = min(h, w) - 40
    y1 = (h - crop_size) // 2
    x1 = (w - crop_size) // 2
    crop = frame[y1:y1+crop_size, x1:x1+crop_size]
    cv2.rectangle(display, (x1, y1), (x1+crop_size, y1+crop_size),
                  (0, 255, 0), 2)

    # Resize ke input model
    img = cv2.resize(crop, (IMG_SIZE, IMG_SIZE))
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    if model_loaded:
        # Inference
        input_data = np.expand_dims(img_rgb, axis=0)

        if input_details[0]['dtype'] == np.int8:
            input_scale, input_zero = input_details[0]['quantization']
            input_data = (input_data / input_scale + input_zero).astype(np.int8)
        else:
            input_data = input_data.astype(np.float32) / 255.0

        interpreter.set_tensor(input_details[0]['index'], input_data)
        interpreter.invoke()
        output = interpreter.get_tensor(output_details[0]['index'])[0]

        if output_details[0]['dtype'] == np.int8:
            output_scale, output_zero = output_details[0]['quantization']
            output = (output.astype(np.float32) - output_zero) * output_scale

        # Softmax
        exp_out = np.exp(output - np.max(output))
        probs = exp_out / exp_out.sum()
    else:
        probs = np.random.dirichlet(np.ones(len(LABELS)))

    # Display results
    best_idx = np.argmax(probs)
    best_label = LABELS[best_idx]
    best_conf = probs[best_idx]

    color = (0, 255, 0) if best_conf > 0.7 else (0, 255, 255)
    cv2.putText(display, f"{best_label}: {best_conf:.0%}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 1, color, 2)

    # Bar chart
    for i, (label, prob) in enumerate(zip(LABELS, probs)):
        bar_w = int(prob * 200)
        y = 60 + i * 25
        cv2.rectangle(display, (10, y), (10 + bar_w, y + 18),
                      (0, int(prob * 255), 0), -1)
        cv2.putText(display, f"{label}: {prob:.0%}", (220, y + 15),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

    # Preview 96x96
    preview = cv2.resize(img, (150, 150), interpolation=cv2.INTER_NEAREST)
    display[10:160, w-160:w-10] = preview

    cv2.imshow("Image Classifier", display)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
```

---

## 📝 Latihan Mandiri

1. **Collect** 50+ gambar per kelas (3 kelas minimal)
2. **Train** di Edge Impulse dengan MobileNetV2 96x96 0.1
3. **Bandingkan** akurasi float32 vs INT8 quantized
4. **Deploy** ke ESP32-CAM dan test klasifikasi real-time
5. **Challenge**: Buat 5 kelas sampah (plastik, kertas, organik, logam, kaca)

---

## 📚 Referensi
- [Edge Impulse Image Classification](https://docs.edgeimpulse.com/docs/tutorials/image-classification)
- [MobileNet Paper](https://arxiv.org/abs/1801.04381)
- [Transfer Learning Explained](https://www.tensorflow.org/tutorials/images/transfer_learning)
- [ESP32-CAM + Edge Impulse](https://docs.edgeimpulse.com/docs/development-platforms/officially-supported-mcu-targets/espressif-esp-eye)
