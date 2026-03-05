# Sesi 25 — Edge Impulse: Motion/Gesture Classification (Accelerometer)

## 🎯 Tujuan Pembelajaran
- Menghubungkan sensor MPU6050 (accelerometer + gyroscope) ke ESP32
- Mengumpulkan data gesture dari akselerometer
- Training model gesture classifier di Edge Impulse
- Deploy gesture recognition on-device (tanpa PC!)
- 🔨 Mini Project: Magic Wand — kontrol LED dengan gerakan tangan

---

## 📖 Teori

### Apa itu Accelerometer?

**Accelerometer** = sensor yang mengukur percepatan (acceleration) termasuk gravitasi.

```
Bayangkan Anda memegang HP:

📱 Diam di meja:
   X = 0, Y = 0, Z = 9.8 m/s²  (gravitasi bumi ke bawah)

📱 Dimiringkan ke kanan:
   X = 5.0, Y = 0, Z = 8.5      (ada komponen gravitasi ke samping)

📱 Digoyang-goyang:
   X = berubah cepat!             (ada percepatan bolak-balik)

💡 Setiap gerakan menghasilkan POLA UNIK pada data X, Y, Z!
```

### MPU6050 Sensor

MPU6050 = sensor **6-axis** (3 axis accelerometer + 3 axis gyroscope) yang sangat populer dan murah (~Rp 15.000).

```
MPU6050 mengukur:
  ACCELEROMETER (percepatan):        GYROSCOPE (kecepatan rotasi):
  ─────────────────────────          ────────────────────────────
  AccX → kiri/kanan                  GyroX → rotasi pitch (angguk)
  AccY → maju/mundur                 GyroY → rotasi roll (miring)
  AccZ → atas/bawah                  GyroZ → rotasi yaw (putar)

WIRING KE ESP32:
  ┌──────────────────────────┐
  │ MPU6050    │   ESP32     │
  │ VCC        │   3.3V      │
  │ GND        │   GND       │
  │ SCL        │   GPIO 22   │  ← I2C Clock
  │ SDA        │   GPIO 21   │  ← I2C Data
  │ INT        │   (opsional)│
  │ AD0        │   GND       │  ← Address 0x68
  └──────────────────────────┘
```

### Gesture Recognition Pipeline

```
┌───────────────────────────────────────────────────────────────────┐
│  Gerakkan ESP32    →  MPU6050 baca    →  Kirim ke        →  ML  │
│  dengan tangan        AccX/Y/Z             Edge Impulse      ↓  │
│                        GyroX/Y/Z                            Train│
│                                                               ↓  │
│  LED response    ←  ESP32 inference  ←  Deploy model     ← Done │
└───────────────────────────────────────────────────────────────────┘
```

---

## 🛠️ Praktik

### 1. [ESP32] Baca Data MPU6050

```cpp
/**
 * sesi25_mpu6050_basic.ino
 * Baca data accelerometer dan gyroscope dari MPU6050
 *
 * WIRING:
 *   MPU6050 VCC → 3.3V
 *   MPU6050 GND → GND
 *   MPU6050 SCL → GPIO 22
 *   MPU6050 SDA → GPIO 21
 *
 * Install library:
 *   PlatformIO: lib_deps = adafruit/Adafruit MPU6050
 *   Arduino: Library Manager → Adafruit MPU6050
 */
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);  // SDA=21, SCL=22

    Serial.println("Initializing MPU6050...");

    if (!mpu.begin()) {
        Serial.println("MPU6050 not found! Check wiring.");
        while (1) delay(10);
    }

    // Konfigurasi
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);   // ±8g
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);        // ±500°/s
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);     // Low-pass filter

    Serial.println("MPU6050 ready! ✅");
    Serial.println("AccX, AccY, AccZ, GyroX, GyroY, GyroZ");
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Print data
    Serial.printf("Acc: %.2f, %.2f, %.2f | Gyro: %.2f, %.2f, %.2f\n",
                   a.acceleration.x, a.acceleration.y, a.acceleration.z,
                   g.gyro.x, g.gyro.y, g.gyro.z);

    delay(20);  // 50 Hz
}
```

### 2. [ESP32] Data Forwarder untuk Edge Impulse

```cpp
/**
 * sesi25_mpu_forwarder.ino
 * Kirim data MPU6050 ke Edge Impulse via Data Forwarder
 *
 * Setelah upload:
 * 1. Jalankan: edge-impulse-data-forwarder
 * 2. Pilih project dan set sensor names:
 *    accX, accY, accZ, gyrX, gyrY, gyrZ
 * 3. Collect data dari Edge Impulse Studio
 */
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

#define INTERVAL_MS 20  // 50 Hz sampling

unsigned long lastSample = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);

    if (!mpu.begin()) {
        Serial.println("MPU6050 error!");
        while (1);
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    delay(500);
}

void loop() {
    if (millis() - lastSample >= INTERVAL_MS) {
        lastSample = millis();

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Format CSV: accX,accY,accZ,gyrX,gyrY,gyrZ
        // Data Forwarder akan parsing format ini
        Serial.printf("%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                       a.acceleration.x, a.acceleration.y, a.acceleration.z,
                       g.gyro.x, g.gyro.y, g.gyro.z);
    }
}
```

### 3. Collect Gesture Data di Edge Impulse

```
GESTURE YANG AKAN KITA BUAT:

1. "circle"  ⭕ = Gerakkan ESP32 dalam lingkaran (2 detik)
2. "wave"    👋 = Goyangkan ESP32 kiri-kanan (2 detik)
3. "shake"   🤝 = Kocok ESP32 naik-turun cepat (2 detik)
4. "idle"    😐 = ESP32 diam di meja (2 detik)

CARA COLLECT DATA:

1. Flash firmware sesi25_mpu_forwarder.ino ke ESP32
2. Jalankan: edge-impulse-data-forwarder
   → Connect ke project Anda
   → Set labels: accX, accY, accZ, gyrX, gyrY, gyrZ

3. Buka Edge Impulse Studio → Data acquisition
4. Set:
   - Label: "circle"
   - Sample length: 2000 ms
   - Sensor: semua 6 axis
5. Klik "Start sampling"
   → Saat countdown mulai, gerakkan ESP32 dalam lingkaran
6. Ulangi 30-50x per gesture

TIPS PENTING:
✅ Pegang ESP32 dengan orientasi SAMA setiap kali
✅ Variasikan kecepatan (cepat, sedang, lambat)
✅ Tangan kiri DAN kanan (jika beda user)
✅ "idle" sangat penting — ini baseline
❌ Jangan gerakkan terlalu pelan (data mirip idle)
❌ Jangan terlalu cepat (data saturasi)
```

### 4. Design Impulse & Training

```
DI EDGE IMPULSE STUDIO:

1. CREATE IMPULSE:
   - Time series data: Window size = 2000ms, Window increase = 80ms
   - Processing block: "Spectral Analysis"
     (Mengubah data time-series menjadi frequency features)
   - Learning block: "Classification"
   - Save impulse

2. SPECTRAL FEATURES:
   - Klik "Spectral features" di sidebar
   - Parameter default biasanya OK:
     - Type: FFT
     - Scale axes: checked
   - Klik "Save parameters"
   - Klik "Generate features"
   - Lihat Feature explorer → cluster harus terpisah!

3. TRAINING:
   - Klik "Classifier" (NN Classifier)
   - Architecture:
     Input → Dense(20, relu) → Dense(10, relu) → Output(4, softmax)
   - Training cycles: 100
   - Learning rate: 0.005
   - Klik "Start training"
   - Target: Accuracy > 85%

4. MODEL TESTING:
   - Klik "Model testing"
   - Klik "Classify all"
   - Lihat akurasi pada data test set
```

### 5. 🔨 Mini Project: Magic Wand

```cpp
/**
 * sesi25_magic_wand.ino
 * 🔨 MINI PROJECT: Magic Wand — kontrol LED dengan gesture
 *
 * Gesture "circle" ⭕ → nyalakan LED
 * Gesture "wave"   👋 → matikan LED
 * Gesture "shake"  🤝 → LED blink 3x (konfirmasi)
 * Gesture "idle"   😐 → tidak ada aksi
 *
 * SEBELUM UPLOAD:
 * 1. Train model di Edge Impulse
 * 2. Deploy sebagai Arduino library
 * 3. Install library di Arduino IDE
 * 4. Ganti #include dengan nama project Anda
 */

// GANTI DENGAN LIBRARY ANDA!
#include <YOUR_PROJECT_NAME_inferencing.h>

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define LED_PIN  2     // Built-in LED
#define LED2_PIN 4     // LED tambahan (opsional)

Adafruit_MPU6050 mpu;
bool ledState = false;

// Buffer untuk data sensor
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
int featureIndex = 0;

// Sampling
#define INTERVAL_MS 20  // 50 Hz
unsigned long lastSample = 0;
bool collecting = false;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);

    Wire.begin(21, 22);

    if (!mpu.begin()) {
        Serial.println("MPU6050 error!");
        while (1);
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("==============================");
    Serial.println("   🪄 Magic Wand Ready!");
    Serial.println("   ⭕ Circle → LED ON");
    Serial.println("   👋 Wave   → LED OFF");
    Serial.println("   🤝 Shake  → Blink confirm");
    Serial.println("==============================");
}

void blinkLED(int times) {
    for (int i = 0; i < times; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    // Restore state
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void runInference() {
    // Buat signal dari buffer
    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

    // Jalankan klasifikasi
    ei_impulse_result_t result;
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
        Serial.printf("Classifier error: %d\n", err);
        return;
    }

    // Cari prediksi tertinggi
    float maxVal = 0;
    int maxIdx = 0;
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        Serial.printf("  %s: %.2f  ", ei_classifier_inferencing_categories[i],
                       result.classification[i].value);
        if (result.classification[i].value > maxVal) {
            maxVal = result.classification[i].value;
            maxIdx = i;
        }
    }

    String gesture = ei_classifier_inferencing_categories[maxIdx];
    Serial.printf("\n  → %s (%.0f%%)\n", gesture.c_str(), maxVal * 100);

    // Aksi berdasarkan gesture (confidence > 70%)
    if (maxVal > 0.70) {
        if (gesture == "circle") {
            ledState = true;
            digitalWrite(LED_PIN, HIGH);
            Serial.println("  ⭕ Circle → LED ON! 💡");
        }
        else if (gesture == "wave") {
            ledState = false;
            digitalWrite(LED_PIN, LOW);
            Serial.println("  👋 Wave → LED OFF! 🌑");
        }
        else if (gesture == "shake") {
            Serial.println("  🤝 Shake → Blink!");
            blinkLED(3);
        }
        // "idle" → no action
    }

    Serial.printf("  Timing — DSP: %dms, NN: %dms\n",
                   result.timing.dsp, result.timing.classification);
}

void loop() {
    if (millis() - lastSample >= INTERVAL_MS) {
        lastSample = millis();

        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Masukkan data ke buffer (6 values per sample)
        if (featureIndex < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE - 5) {
            features[featureIndex++] = a.acceleration.x;
            features[featureIndex++] = a.acceleration.y;
            features[featureIndex++] = a.acceleration.z;
            features[featureIndex++] = g.gyro.x;
            features[featureIndex++] = g.gyro.y;
            features[featureIndex++] = g.gyro.z;
        }

        // Buffer penuh → inference!
        if (featureIndex >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            runInference();
            featureIndex = 0;  // Reset buffer
        }
    }
}
```

---

## 📊 Expected Results

```
Setelah training:
  Accuracy: ~80-92%
  Model size: ~15-30 KB
  Inference time: ~10-20 ms

Confusion Matrix (contoh):
           circle   wave   shake   idle
  circle    88%      3%     5%     4%
  wave       2%    90%      5%     3%
  shake      1%      4%    91%     4%
  idle       2%      1%     3%    94%
```

---

## 📝 Latihan Mandiri

1. **Wiring MPU6050** dan jalankan firmware basic — lihat data berubah saat gerakkan
2. **Collect** 30+ sample per gesture (circle, wave, shake, idle)
3. **Train** model — aim for >85% accuracy
4. **Deploy** dan test Magic Wand — coba dari jarak berbeda, kecepatan berbeda
5. **Tambah gesture** baru: "flick" (sentilan) untuk toggle mode

---

## 📚 Referensi
- [Edge Impulse Motion Classification](https://docs.edgeimpulse.com/docs/tutorials/continuous-motion-recognition)
- [MPU6050 Guide](https://randomnerdtutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/)
- [Spectral Analysis](https://docs.edgeimpulse.com/docs/edge-impulse-studio/processing-blocks/spectral-features)
