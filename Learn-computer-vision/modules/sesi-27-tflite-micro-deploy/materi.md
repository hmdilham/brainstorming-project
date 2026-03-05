# Sesi 27 — TensorFlow Lite Micro: Deploy Manual ke ESP32

## 🎯 Tujuan Pembelajaran
- Memahami TensorFlow Lite Micro (TFLite Micro) runtime
- Pipeline manual: Train di PC → Convert → Quantize → Embed ke ESP32
- Menggunakan `xxd` untuk mengubah model menjadi C array
- Menulis inference loop di ESP32 dari nol
- 🔨 Mini Project: On-Device Anomaly Detector (getaran mesin)

---

## 📖 Teori

### Kenapa Perlu Deploy Manual?

Edge Impulse sangat membantu, tapi ada saatnya Anda perlu deploy **manual**:
- Model custom yang tidak bisa dibuat di Edge Impulse
- Ingin kontrol penuh atas preprocessing dan postprocessing
- Project yang butuh optimasi khusus
- Belajar memahami apa yang terjadi "di balik layar"

### Pipeline Manual vs Edge Impulse

```
EDGE IMPULSE (otomatis):
  Data → [Edge Impulse handles everything] → Arduino library → Upload
  ↑ Mudah, tapi kurang fleksibel

MANUAL (TFLite Micro):
  1. Train model di PC (Python/Keras)
  2. Convert ke TFLite (.tflite)
  3. Quantize ke INT8
  4. Convert ke C array (xxd)
  5. Tulis kode inference ESP32
  6. Upload
  ↑ Lebih sulit, tapi kontrol penuh!
```

### Arsitektur TensorFlow Lite Micro

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32 Memory Layout                        │
│                                                             │
│  ┌─────────────────┐  ┌──────────────────────────────────┐ │
│  │    FLASH (4MB)   │  │           SRAM (520KB)          │ │
│  │                  │  │                                  │ │
│  │  ┌────────────┐  │  │  ┌──────────────┐  ┌─────────┐ │ │
│  │  │ Firmware   │  │  │  │ Tensor Arena │  │ Program │ │ │
│  │  │ (kode Anda)│  │  │  │ (workspace   │  │ Stack   │ │ │
│  │  │ + model    │  │  │  │  untuk model)│  │ & Heap  │ │ │
│  │  │ (.tflite   │  │  │  │ ~50-200 KB   │  │         │ │ │
│  │  │  as C array│  │  │  │              │  │         │ │ │
│  │  └────────────┘  │  │  └──────────────┘  └─────────┘ │ │
│  └─────────────────┘  └──────────────────────────────────┘ │
│                                                             │
│  Model disimpan di FLASH (read-only),                       │
│  Tensor Arena di RAM (read-write, untuk kalkulasi)          │
└─────────────────────────────────────────────────────────────┘
```

### Apa itu Tensor Arena?

```
Tensor Arena = "ruang kerja" di RAM tempat TFLite Micro
melakukan semua perhitungan.

Analogi:
  Model = buku resep (disimpan di rak = Flash)
  Tensor Arena = meja dapur (tempat memasak = RAM)

  Meja terlalu kecil? → Tidak bisa masak semua sekaligus → ERROR
  Meja cukup besar?   → Bisa masak dengan lancar → OK!

Ukuran Tensor Arena tergantung model:
  Model kecil (keyword spotting): ~10-30 KB
  Model sedang (image classifier): ~50-150 KB
  Model besar: → tidak muat di ESP32!
```

---

## 🛠️ Praktik

### STEP 1: Train Model di PC

```python
"""
sesi27_step1_train_model.py
STEP 1: Buat dan train model anomaly detection untuk data akselerometer

Use case: Deteksi getaran abnormal pada mesin
- Normal: getaran halus, amplitudo kecil
- Anomaly: getaran kasar, amplitudo besar/tidak beraturan

Kita bikin model yang SANGAT kecil agar muat di ESP32
"""
import numpy as np
import tensorflow as tf
from tensorflow import keras
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split

# ====== 1. Generate Dataset (Simulasi Data Sensor) ======
print("=" * 50)
print("STEP 1: Generate Dataset")
print("=" * 50)

np.random.seed(42)
SAMPLES_PER_CLASS = 500
WINDOW_SIZE = 50   # 50 data points per window (1 detik @ 50Hz)
NUM_FEATURES = 3   # AccX, AccY, AccZ

def generate_normal(n):
    """Getaran normal: sinusoidal halus + sedikit noise."""
    samples = []
    for _ in range(n):
        t = np.linspace(0, 1, WINDOW_SIZE)
        x = np.sin(2 * np.pi * 5 * t) * 0.5 + np.random.randn(WINDOW_SIZE) * 0.1
        y = np.sin(2 * np.pi * 5 * t + 0.5) * 0.3 + np.random.randn(WINDOW_SIZE) * 0.1
        z = 9.8 + np.random.randn(WINDOW_SIZE) * 0.1
        samples.append(np.column_stack([x, y, z]))
    return np.array(samples)

def generate_anomaly(n):
    """Getaran abnormal: amplitudo besar, frekuensi tidak beraturan."""
    samples = []
    for _ in range(n):
        t = np.linspace(0, 1, WINDOW_SIZE)
        freq = np.random.uniform(10, 30)
        amp = np.random.uniform(2, 5)
        x = np.sin(2 * np.pi * freq * t) * amp + np.random.randn(WINDOW_SIZE) * 0.5
        y = np.sin(2 * np.pi * freq * 1.3 * t) * amp * 0.8 + np.random.randn(WINDOW_SIZE) * 0.5
        z = 9.8 + np.sin(2 * np.pi * freq * 0.7 * t) * amp * 0.6 + np.random.randn(WINDOW_SIZE) * 0.3
        samples.append(np.column_stack([x, y, z]))
    return np.array(samples)

X_normal = generate_normal(SAMPLES_PER_CLASS)
X_anomaly = generate_anomaly(SAMPLES_PER_CLASS)

X = np.vstack([X_normal, X_anomaly])
y = np.array([0] * SAMPLES_PER_CLASS + [1] * SAMPLES_PER_CLASS)

# Reshape: (samples, WINDOW_SIZE * NUM_FEATURES)
X_flat = X.reshape(len(X), -1)

print(f"Total samples: {len(X)}")
print(f"Input shape per sample: {X[0].shape} → flattened: {X_flat[0].shape}")
print(f"Normal: {SAMPLES_PER_CLASS}, Anomaly: {SAMPLES_PER_CLASS}")

# Split
X_train, X_test, y_train, y_test = train_test_split(
    X_flat, y, test_size=0.2, random_state=42
)
print(f"Train: {len(X_train)}, Test: {len(X_test)}")

# ====== 2. Build Model (SANGAT KECIL!) ======
print("\n" + "=" * 50)
print("STEP 2: Build Tiny Model")
print("=" * 50)

INPUT_SIZE = WINDOW_SIZE * NUM_FEATURES  # 50 * 3 = 150

model = keras.Sequential([
    keras.layers.Input(shape=(INPUT_SIZE,)),
    keras.layers.Dense(16, activation='relu'),      # 16 neuron
    keras.layers.Dense(8, activation='relu'),       # 8 neuron
    keras.layers.Dense(1, activation='sigmoid'),    # output: 0 atau 1
])

model.compile(
    optimizer=keras.optimizers.Adam(learning_rate=0.001),
    loss='binary_crossentropy',
    metrics=['accuracy']
)

model.summary()
print(f"\nTotal parameters: {model.count_params()}")
print(f"Estimated model size: ~{model.count_params() * 4 / 1024:.1f} KB (float32)")
print(f"After quantization: ~{model.count_params() / 1024:.1f} KB (int8)")

# ====== 3. Training ======
print("\n" + "=" * 50)
print("STEP 3: Training")
print("=" * 50)

history = model.fit(
    X_train, y_train,
    epochs=30,
    batch_size=32,
    validation_split=0.2,
    verbose=1
)

# Evaluasi
test_loss, test_acc = model.evaluate(X_test, y_test, verbose=0)
print(f"\n✅ Test Accuracy: {test_acc:.1%}")

# Save Keras model
model.save("anomaly_model.h5")
print("Saved: anomaly_model.h5")

# Plot training history
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4))
ax1.plot(history.history['loss'], label='Train Loss')
ax1.plot(history.history['val_loss'], label='Val Loss')
ax1.set_title('Loss'); ax1.legend(); ax1.grid(True, alpha=0.3)
ax2.plot(history.history['accuracy'], label='Train Acc')
ax2.plot(history.history['val_accuracy'], label='Val Acc')
ax2.set_title('Accuracy'); ax2.legend(); ax2.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("training_history.png")
plt.show()
```

### STEP 2: Convert ke TFLite + Quantize

```python
"""
sesi27_step2_convert_tflite.py
STEP 2: Convert model ke TFLite dan quantize INT8
"""
import tensorflow as tf
import numpy as np
import os

# ====== Load model ======
model = tf.keras.models.load_model("anomaly_model.h5")
print(f"Loaded model: {model.count_params()} parameters")

# ====== Representative dataset (untuk quantization) ======
# Kita perlu contoh data untuk kalibrasi quantization
np.random.seed(42)
WINDOW_SIZE = 50
NUM_FEATURES = 3

def representative_dataset():
    """Generate sample data untuk kalibrasi INT8."""
    for _ in range(100):
        t = np.linspace(0, 1, WINDOW_SIZE)
        x = np.sin(2 * np.pi * 5 * t) * 0.5 + np.random.randn(WINDOW_SIZE) * 0.1
        y = np.sin(2 * np.pi * 5 * t) * 0.3 + np.random.randn(WINDOW_SIZE) * 0.1
        z = 9.8 + np.random.randn(WINDOW_SIZE) * 0.1
        sample = np.column_stack([x, y, z]).flatten()
        yield [sample.reshape(1, -1).astype(np.float32)]

# ====== Convert: Float32 TFLite ======
print("\n--- Converting to TFLite Float32 ---")
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_float = converter.convert()

with open("anomaly_float32.tflite", "wb") as f:
    f.write(tflite_float)

# ====== Convert: INT8 Quantized TFLite ======
print("--- Converting to TFLite INT8 ---")
converter_quant = tf.lite.TFLiteConverter.from_keras_model(model)
converter_quant.optimizations = [tf.lite.Optimize.DEFAULT]
converter_quant.representative_dataset = representative_dataset
converter_quant.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter_quant.inference_input_type = tf.int8
converter_quant.inference_output_type = tf.int8

tflite_int8 = converter_quant.convert()

with open("anomaly_int8.tflite", "wb") as f:
    f.write(tflite_int8)

# ====== Summary ======
print("\n" + "=" * 50)
print("CONVERSION SUMMARY")
print("=" * 50)
keras_size = os.path.getsize("anomaly_model.h5")
f32_size = len(tflite_float)
i8_size = len(tflite_int8)

print(f"Keras model:      {keras_size:>8d} bytes ({keras_size/1024:.1f} KB)")
print(f"TFLite Float32:   {f32_size:>8d} bytes ({f32_size/1024:.1f} KB)")
print(f"TFLite INT8:      {i8_size:>8d} bytes ({i8_size/1024:.1f} KB)")
print(f"\nKompresi: {(1 - i8_size/keras_size)*100:.0f}% lebih kecil dari Keras!")
print(f"INT8 model: {i8_size/1024:.1f} KB → {'✅ Muat di ESP32!' if i8_size < 200*1024 else '❌ Terlalu besar'}")
```

### STEP 3: Convert ke C Array (xxd)

```python
"""
sesi27_step3_to_c_array.py
STEP 3: Convert file .tflite menjadi C array
sehingga bisa di-embed langsung di firmware ESP32

Ini adalah langkah yang dilakukan Edge Impulse otomatis,
tapi kita lakukan manual untuk pemahaman lebih dalam.
"""
import os

def tflite_to_c_array(tflite_path, output_path, array_name="model_data"):
    """Convert file .tflite menjadi file .h berisi C array."""

    with open(tflite_path, "rb") as f:
        data = f.read()

    # Header
    lines = []
    lines.append(f"// Auto-generated from {os.path.basename(tflite_path)}")
    lines.append(f"// Model size: {len(data)} bytes ({len(data)/1024:.1f} KB)")
    lines.append(f"#ifndef {array_name.upper()}_H")
    lines.append(f"#define {array_name.upper()}_H")
    lines.append("")
    lines.append(f"const unsigned int {array_name}_len = {len(data)};")
    lines.append(f"alignas(8) const unsigned char {array_name}[] = {{")

    # Data (16 bytes per line)
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ", ".join([f"0x{b:02x}" for b in chunk])
        if i + 16 < len(data):
            hex_str += ","
        lines.append(f"  {hex_str}")

    lines.append("};")
    lines.append("")
    lines.append(f"#endif  // {array_name.upper()}_H")

    with open(output_path, "w") as f:
        f.write("\n".join(lines))

    print(f"✅ Generated: {output_path}")
    print(f"   Array name: {array_name}")
    print(f"   Size: {len(data)} bytes ({len(data)/1024:.1f} KB)")
    print(f"   Lines: {len(lines)}")

    return output_path

# Convert
tflite_to_c_array("anomaly_int8.tflite", "anomaly_model_data.h", "anomaly_model")

print()
print("Sekarang copy file 'anomaly_model_data.h' ke folder")
print("project PlatformIO/Arduino Anda.")
print()
print("Alternatif menggunakan xxd (Linux/Mac):")
print("  xxd -i anomaly_int8.tflite > anomaly_model_data.h")
```

### STEP 4: ESP32 Firmware dengan TFLite Micro

```cpp
/**
 * sesi27_tflite_inference.ino
 * STEP 4: Jalankan inference TFLite Micro di ESP32
 *
 * 🔨 MINI PROJECT: On-Device Anomaly Detector
 * Deteksi getaran abnormal mesin dari akselerometer MPU6050
 *
 * WIRING:
 *   MPU6050 VCC → 3.3V
 *   MPU6050 GND → GND
 *   MPU6050 SCL → GPIO 22
 *   MPU6050 SDA → GPIO 21
 *   LED (normal) → GPIO 2 (hijau)
 *   LED (anomaly) → GPIO 4 (merah)
 *   Buzzer → GPIO 5
 *
 * LIBRARY YANG DIBUTUHKAN:
 *   - TensorFlowLite_ESP32 (PlatformIO: lib_deps = tanakamasayuki/TensorFlowLite_ESP32)
 *   - Adafruit MPU6050
 *
 * platformio.ini:
 *   [env:esp32dev]
 *   platform = espressif32
 *   board = esp32dev
 *   framework = arduino
 *   lib_deps =
 *     tanakamasayuki/TensorFlowLite_ESP32
 *     adafruit/Adafruit MPU6050
 */

#include <TensorFlowLite_ESP32.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Model data (generated by step 3)
#include "anomaly_model_data.h"

// Pins
#define LED_NORMAL  2
#define LED_ANOMALY 4
#define BUZZER_PIN  5

// Model config
#define WINDOW_SIZE    50        // Harus sama dengan training
#define NUM_FEATURES   3         // AccX, AccY, AccZ
#define INPUT_SIZE     (WINDOW_SIZE * NUM_FEATURES)  // 150

// TFLite Micro
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Tensor Arena — sesuaikan ukuran berdasarkan model
constexpr int kTensorArenaSize = 8 * 1024;  // 8 KB (mulai kecil, tambah jika error)
uint8_t tensor_arena[kTensorArenaSize];

// Sensor
Adafruit_MPU6050 mpu;

// Data buffer
float sensorBuffer[INPUT_SIZE];
int bufferIndex = 0;

// Timing
#define SAMPLE_INTERVAL_MS 20   // 50 Hz
unsigned long lastSample = 0;

bool anomalyDetected = false;
int anomalyCount = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Pins
    pinMode(LED_NORMAL, OUTPUT);
    pinMode(LED_ANOMALY, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(LED_NORMAL, HIGH);   // Hijau ON = system ready
    digitalWrite(LED_ANOMALY, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // ====== MPU6050 ======
    Wire.begin(21, 22);
    if (!mpu.begin()) {
        Serial.println("❌ MPU6050 not found!");
        while (1);
    }
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("✅ MPU6050 ready");

    // ====== TFLite Micro Setup ======
    Serial.println("Loading TFLite model...");

    // 1. Load model
    model = tflite::GetModel(anomaly_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.printf("Model version mismatch: %d vs %d\n",
                       model->version(), TFLITE_SCHEMA_VERSION);
        while (1);
    }

    // 2. Resolver (mendaftarkan semua operasi yang bisa digunakan)
    static tflite::AllOpsResolver resolver;

    // 3. Interpreter
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize
    );
    interpreter = &static_interpreter;

    // 4. Allocate tensors
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        Serial.println("❌ AllocateTensors failed!");
        Serial.println("   Coba perbesar kTensorArenaSize");
        while (1);
    }

    // 5. Dapatkan pointer input/output
    input = interpreter->input(0);
    output = interpreter->output(0);

    // Print info
    Serial.println("✅ TFLite model loaded!");
    Serial.printf("   Input:  type=%d, dims=[%d, %d]\n",
                   input->type, input->dims->data[0], input->dims->data[1]);
    Serial.printf("   Output: type=%d, dims=[%d, %d]\n",
                   output->type, output->dims->data[0], output->dims->data[1]);
    Serial.printf("   Arena used: %d bytes\n",
                   interpreter->arena_used_bytes());

    Serial.println("\n==============================");
    Serial.println("  🔍 Anomaly Detector Active");
    Serial.println("  Green LED = Normal");
    Serial.println("  Red LED + Buzzer = Anomaly!");
    Serial.println("==============================\n");
}

void runInference() {
    // --- Masukkan data ke input tensor ---
    if (input->type == kTfLiteInt8) {
        // INT8 quantized: perlu scale & zero point
        float input_scale = input->params.scale;
        int32_t input_zero = input->params.zero_point;

        for (int i = 0; i < INPUT_SIZE; i++) {
            int8_t quantized = (int8_t)(sensorBuffer[i] / input_scale + input_zero);
            input->data.int8[i] = quantized;
        }
    } else {
        // Float32
        for (int i = 0; i < INPUT_SIZE; i++) {
            input->data.f[i] = sensorBuffer[i];
        }
    }

    // --- Jalankan inference ---
    unsigned long start = micros();
    TfLiteStatus invoke_status = interpreter->Invoke();
    unsigned long elapsed = micros() - start;

    if (invoke_status != kTfLiteOk) {
        Serial.println("Invoke failed!");
        return;
    }

    // --- Baca output ---
    float prediction;
    if (output->type == kTfLiteInt8) {
        float output_scale = output->params.scale;
        int32_t output_zero = output->params.zero_point;
        prediction = (output->data.int8[0] - output_zero) * output_scale;
    } else {
        prediction = output->data.f[0];
    }

    // --- Proses hasil ---
    // prediction > 0.5 = anomaly, < 0.5 = normal
    bool isAnomaly = prediction > 0.5;

    if (isAnomaly && !anomalyDetected) {
        anomalyDetected = true;
        anomalyCount++;
        digitalWrite(LED_ANOMALY, HIGH);
        digitalWrite(LED_NORMAL, LOW);
        Serial.printf("⚠️ ANOMALY #%d! (%.1f%%, %lu µs)\n",
                       anomalyCount, prediction * 100, elapsed);
        // Buzzer beep
        tone(BUZZER_PIN, 2000, 500);
    }
    else if (!isAnomaly && anomalyDetected) {
        anomalyDetected = false;
        digitalWrite(LED_ANOMALY, LOW);
        digitalWrite(LED_NORMAL, HIGH);
        noTone(BUZZER_PIN);
        Serial.printf("✅ Normal (%.1f%%, %lu µs)\n",
                       prediction * 100, elapsed);
    }

    // Debug (setiap 5 detik)
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 5000) {
        Serial.printf("Status: %s | Score: %.1f%% | Inference: %lu µs | Anomalies: %d\n",
                       isAnomaly ? "ANOMALY" : "NORMAL",
                       prediction * 100, elapsed, anomalyCount);
        lastDebug = millis();
    }
}

void loop() {
    if (millis() - lastSample >= SAMPLE_INTERVAL_MS) {
        lastSample = millis();

        // Baca sensor
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);

        // Masukkan ke buffer
        sensorBuffer[bufferIndex++] = a.acceleration.x;
        sensorBuffer[bufferIndex++] = a.acceleration.y;
        sensorBuffer[bufferIndex++] = a.acceleration.z;

        // Buffer penuh → inference!
        if (bufferIndex >= INPUT_SIZE) {
            runInference();
            bufferIndex = 0;
        }
    }
}
```

---

## 📊 Ringkasan Pipeline

```
┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐     ┌─────────┐
│  Train   │────►│ Convert │────►│Quantize │────►│ xxd /   │────►│ ESP32   │
│  Keras   │     │ TFLite  │     │ INT8    │     │ C array │     │Firmware │
│  (PC)    │     │ (.tflite│     │         │     │ (.h)    │     │         │
│          │     │  ~5KB)  │     │ (~2KB)  │     │         │     │ Run!    │
└─────────┘     └─────────┘     └─────────┘     └─────────┘     └─────────┘
```

---

## 📝 Latihan Mandiri

1. **Jalankan step 1-3** di PC: train → convert → C array
2. **Baca ukuran** model di setiap step — bandingkan
3. **Flash ke ESP32** dan test dengan data sensor MPU6050
4. **Goyangkan ESP32** dengan intensitas berbeda — kapan model mendeteksi anomaly?
5. **Challenge**: Ubah model untuk 3 kelas (normal, anomaly_ringan, anomaly_berat)

---

## 📚 Referensi
- [TFLite Micro Guide](https://www.tensorflow.org/lite/microcontrollers)
- [TFLite Micro ESP32](https://github.com/tanakamasayuki/Arduino_TensorFlowLite_ESP32)
- [Post-Training Quantization](https://www.tensorflow.org/lite/performance/post_training_quantization)
- [Model Optimization](https://www.tensorflow.org/lite/performance/model_optimization)
