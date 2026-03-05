# Sesi 24 — Edge Impulse: Audio Classification (Keyword Spotting)

## 🎯 Tujuan Pembelajaran
- Memahami bagaimana komputer "mendengar" suara (audio features)
- Menggunakan Edge Impulse untuk membuat keyword spotter
- Deploy keyword spotting di ESP32 (tanpa internet!)
- 🔨 Mini Project: Voice-Controlled LED — "nyala" / "mati"

---

## 📖 Teori

### Bagaimana Komputer "Mendengar"?

Manusia: Suara masuk telinga → otak kenali → "Oh, itu kata 'nyala'"

Komputer: Suara masuk mikrofon → diubah jadi **angka** → dicari **pola** → dicocokkan dengan model

```
🎤 Mikrofon                 🔢 Digitalisasi              📊 Feature Extraction
  Gelombang suara    →    Angka-angka (sample)    →    MFCC / Spectrogram
  ~~~~~                    [0.1, -0.3, 0.5, ...]         (gambar dari suara)

  📊 MFCC                   🧠 Neural Network            📢 Output
  (ciri-ciri suara)  →    Bandingkan pola       →    "nyala" (95%)
                           dengan yang dipelajari        "mati" (3%)
                                                         "noise" (2%)
```

### Apa itu MFCC?

**MFCC** (Mel-Frequency Cepstral Coefficients) = cara mengubah suara menjadi "sidik jari" yang unik.

```
Analogi:
  Suara "nyala" → MFCC → gambar sidik jari A
  Suara "mati"  → MFCC → gambar sidik jari B
  Noise (kipas) → MFCC → gambar sidik jari C

  Setiap kata punya "sidik jari" suara yang BERBEDA!
  Neural network belajar mengenali perbedaan ini.

Visualisasi MFCC (Spectrogram):
  Frekuensi ↑
  tinggi    │  ░░░░      ░░
            │  ████  ░░  ████
            │  ████  ██  ████
  rendah    │  ████  ██  ████
            └──────────────────► Waktu
              "  n y a l a  "
```

### Apa yang Kita Butuhkan?

```
HARDWARE:
  ┌──────────────┐     ┌──────────────┐
  │ ESP32 DevKit │     │ Mikrofon     │
  │              │─────│ INMP441 atau │
  │              │     │ MAX9814      │
  └──────────────┘     └──────────────┘

  INMP441 (I2S digital mic):           MAX9814 (Analog mic):
  ┌───────────────────────┐             ┌──────────────────────┐
  │ SCK  → GPIO 26        │             │ OUT → GPIO 34 (ADC)  │
  │ WS   → GPIO 25        │             │ VCC → 3.3V           │
  │ SD   → GPIO 33        │             │ GND → GND            │
  │ VCC  → 3.3V           │             │ GAIN → (opsional)    │
  │ GND  → GND            │             └──────────────────────┘
  │ L/R  → GND (left ch)  │
  └───────────────────────┘

  💡 INMP441 (digital) lebih direkomendasikan untuk kualitas audio yang lebih baik
  💡 MAX9814 (analog) lebih mudah wiring-nya

  TIDAK PUNYA MIKROFON?
  Anda bisa collect data dari mikrofon LAPTOP dan upload
  ke Edge Impulse, lalu deploy model-nya ke ESP32 nanti!
```

---

## 🛠️ Praktik

### Workflow Lengkap di Edge Impulse

```
     STEP 1              STEP 2               STEP 3              STEP 4
  ┌──────────┐       ┌──────────┐        ┌──────────┐       ┌──────────┐
  │  Collect │       │  Design  │        │  Train   │       │  Deploy  │
  │  Audio   │──────►│  Impulse │───────►│  Model   │──────►│  to      │
  │  Samples │       │  (MFCC+  │        │  (NN)    │       │  ESP32   │
  │          │       │   NN)    │        │          │       │          │
  └──────────┘       └──────────┘        └──────────┘       └──────────┘
   3 kelas:           Processing:         50 epochs           Arduino
   "nyala"            MFCC                Accuracy 90%+       library
   "mati"             13 coefficients     Confusion matrix    export
   "noise"            window 1000ms
```

### STEP 1: Collect Audio Data

Ada 3 cara collect data audio:

#### Cara A: Dari Browser (Paling Mudah — Pakai Mikrofon Laptop)

```
1. Buka Edge Impulse Studio → project Anda
2. Klik "Data acquisition"
3. Di "Record new data":
   - Device: "Use your computer" (laptop mic)
   - Label: "nyala"
   - Sample length: 2000 ms (2 detik)
   - Category: Training
4. Klik "Start recording" → ucapkan "NYALA" dengan jelas
5. Ulangi 30-50 kali
6. Ganti label ke "mati" → ucapkan "MATI" 30-50 kali
7. Ganti label ke "noise" → rekam suara lingkungan 30-50 kali
   (kipas, AC, keyboard, diam)

TIPS:
  ✅ Variasikan jarak ke mic (dekat, agak jauh)
  ✅ Variasikan kecepatan bicara (cepat, lambat)
  ✅ Minta orang lain juga merekam (variasi suara)
  ✅ Rekam noise dari berbagai sumber
  ❌ Jangan hanya rekam dalam 1 kondisi sempurna
```

#### Cara B: Dari ESP32 + INMP441

```cpp
/**
 * sesi24_audio_forwarder.ino
 * ESP32 + INMP441 I2S mic → Edge Impulse Data Forwarder
 *
 * WIRING INMP441:
 *   SCK  → GPIO 26
 *   WS   → GPIO 25
 *   SD   → GPIO 33
 *   VCC  → 3.3V
 *   GND  → GND
 *   L/R  → GND (left channel)
 */
#include <driver/i2s.h>

// I2S pins
#define I2S_SCK   26
#define I2S_WS    25
#define I2S_SD    33

// Audio config
#define SAMPLE_RATE    16000
#define SAMPLE_BITS    16
#define CHANNELS       1

void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void setup() {
    Serial.begin(115200);
    setupI2S();
    delay(1000);
    Serial.println("Audio forwarder ready");
}

void loop() {
    int16_t buffer[512];
    size_t bytesRead;

    i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
    int samplesRead = bytesRead / sizeof(int16_t);

    // Kirim sebagai CSV ke Data Forwarder
    for (int i = 0; i < samplesRead; i++) {
        Serial.println(buffer[i]);
    }
}
```

#### Cara C: Upload File WAV

```python
"""
sesi24_record_wav.py
Rekam audio dari mikrofon PC dan simpan sebagai WAV
File WAV bisa di-upload manual ke Edge Impulse
"""
import pyaudio
import wave
import os

SAVE_DIR = "audio_samples"
os.makedirs(SAVE_DIR, exist_ok=True)

# Audio settings (harus cocok dengan Edge Impulse)
RATE = 16000       # 16 kHz
CHANNELS = 1       # Mono
FORMAT = pyaudio.paInt16
CHUNK = 1024
DURATION = 2       # 2 detik per sample

p = pyaudio.PyAudio()

def record_sample(label, index):
    """Rekam satu sample audio."""
    filename = f"{SAVE_DIR}/{label}_{index:03d}.wav"

    print(f"  Recording {label} #{index}... ", end="", flush=True)
    input("(Tekan Enter lalu ucapkan) ")

    stream = p.open(format=FORMAT, channels=CHANNELS,
                    rate=RATE, input=True, frames_per_buffer=CHUNK)

    frames = []
    for _ in range(int(RATE / CHUNK * DURATION)):
        data = stream.read(CHUNK)
        frames.append(data)

    stream.stop_stream()
    stream.close()

    # Simpan WAV
    wf = wave.open(filename, 'wb')
    wf.setnchannels(CHANNELS)
    wf.setsampwidth(p.get_sample_size(FORMAT))
    wf.setframerate(RATE)
    wf.writeframes(b''.join(frames))
    wf.close()

    print(f"✅ Saved: {filename}")
    return filename

# === COLLECT DATA ===
print("=== Audio Sample Collector ===")
print("Kita akan rekam 3 kelas suara:\n")

labels = {
    "nyala": "Ucapkan 'NYALA' dengan jelas",
    "mati": "Ucapkan 'MATI' dengan jelas",
    "noise": "Diam / suara background biasa",
}

SAMPLES_PER_CLASS = 20

for label, instruction in labels.items():
    print(f"\n--- Kelas: {label.upper()} ---")
    print(f"    Instruksi: {instruction}")
    print(f"    Akan merekam {SAMPLES_PER_CLASS} sample\n")

    for i in range(SAMPLES_PER_CLASS):
        record_sample(label, i)

p.terminate()

print(f"\n✅ Selesai! Total: {SAMPLES_PER_CLASS * len(labels)} file WAV")
print(f"   Folder: {SAVE_DIR}/")
print(f"\nUpload ke Edge Impulse:")
print(f"  edge-impulse-uploader --category training {SAVE_DIR}/nyala_*.wav --label nyala")
print(f"  edge-impulse-uploader --category training {SAVE_DIR}/mati_*.wav --label mati")
print(f"  edge-impulse-uploader --category training {SAVE_DIR}/noise_*.wav --label noise")
```

> 💡 Install pyaudio: `pip install pyaudio`
> Jika error di Linux: `sudo apt install portaudio19-dev` lalu `pip install pyaudio`

### STEP 2: Design Impulse (di Edge Impulse Studio)

```
Di Edge Impulse Studio:

1. Klik menu "Create impulse" (atau "Impulse design")

2. TIME SERIES DATA block (sudah ada otomatis):
   - Window size: 1000 ms (1 detik — panjang keyword)
   - Window increase: 500 ms (overlap — agar tidak miss keyword)
   - Frequency: 16000 Hz (atau sesuai data collection)

3. Tambahkan PROCESSING block → pilih "MFCC"
   (Mengubah audio mentah menjadi fitur yang bermakna)
   Settings default biasanya sudah bagus:
   - Number of coefficients: 13
   - FFT length: 256
   - Frame length: 0.02s
   - Frame stride: 0.01s

4. Tambahkan LEARNING block → pilih "Classification"
   (Neural network yang akan belajar dari fitur MFCC)

5. Klik "Save impulse"

Hasilnya:
┌──────────┐     ┌──────────┐     ┌──────────┐
│ Time     │────►│ MFCC     │────►│ NN       │
│ series   │     │ (features│     │ Classify │
│ data     │     │  extract)│     │          │
└──────────┘     └──────────┘     └──────────┘
```

### STEP 3: Generate Features & Training

```
GENERATE FEATURES:
1. Klik menu "MFCC" di sidebar
2. Klik "Generate features"
3. Tunggu sampai selesai
4. Lihat "Feature explorer" → data tiap kelas harus terpisah!

   Jika data cluster-nya terpisah jelas:
   ● ● ●       ▲ ▲ ▲           → ✅ Bagus! Model akan mudah belajar
                    ■ ■ ■

   Jika data campur aduk:
   ● ▲ ■ ● ▲ ●
   ▲ ■ ● ▲ ■              → ❌ Kurang bagus, perlu lebih banyak data

TRAINING:
1. Klik menu "Classifier" di sidebar
2. Settings:
   - Number of training cycles: 100 (epoch)
   - Learning rate: 0.005
   - Minimum confidence: 0.6
3. Neural network architecture:
   (Biasanya otomatis, tapi bisa di-tweak)
   1 input layer → 2 hidden layers (20, 10 neurons) → output layer
4. Klik "Start training"
5. Tunggu 1-3 menit
6. Lihat hasil:
   - Accuracy: harus > 85%
   - Confusion matrix: diagonal harus tinggi
   - Model size: harus < 100 KB untuk ESP32
```

### STEP 4: Deploy ke ESP32

```
1. Klik menu "Deployment"
2. Pilih "Arduino library"
3. Pilih optimasi:
   - ✅ Enable EON Compiler (lebih efisien memori)
   - Quantization: INT8 (lebih kecil, sedikit kurang akurat)
4. Klik "Build"
5. Download file .zip

6. Di Arduino IDE:
   Sketch → Include Library → Add .ZIP Library → pilih file .zip

7. Buka contoh:
   File → Examples → [nama project]_inferencing → nano_ble33_sense_microphone
   (Sesuaikan untuk ESP32)
```

### 🔨 Mini Project: Voice-Controlled LED

```cpp
/**
 * sesi24_voice_led.ino
 * 🔨 MINI PROJECT: Kontrol LED dengan suara ON-DEVICE
 *
 * "nyala" → LED ON
 * "mati"  → LED OFF
 * Semua inference di ESP32, TANPA internet!
 *
 * SEBELUM UPLOAD:
 * 1. Sudah train model di Edge Impulse
 * 2. Sudah download Arduino library (.zip)
 * 3. Sudah install library di Arduino IDE
 * 4. Ganti #include dengan nama library project Anda
 *
 * WIRING:
 *   LED → GPIO 2 (built-in) atau GPIO 4
 *   INMP441 I2S mic:
 *     SCK  → GPIO 26
 *     WS   → GPIO 25
 *     SD   → GPIO 33
 *     VCC  → 3.3V
 *     GND  → GND
 */

// GANTI DENGAN NAMA LIBRARY PROJECT ANDA!
#include <YOUR_PROJECT_NAME_inferencing.h>

#include <driver/i2s.h>

// Pin definitions
#define LED_PIN    2
#define I2S_SCK    26
#define I2S_WS     25
#define I2S_SD     33

// Audio buffer
#define SAMPLE_RATE     16000
int16_t sampleBuffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

bool ledState = false;

// I2S setup
void setupI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = false
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    setupI2S();

    Serial.println("=============================");
    Serial.println("  Voice-Controlled LED Ready");
    Serial.println("  Say 'nyala' or 'mati'!");
    Serial.println("=============================");

    // Print model info
    Serial.printf("Model input size: %d samples\n",
                   EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    Serial.printf("Model labels: %d\n",
                   EI_CLASSIFIER_LABEL_COUNT);
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        Serial.printf("  - %s\n", ei_classifier_inferencing_categories[i]);
    }
}

void loop() {
    // 1. Rekam audio dari mikrofon
    size_t bytesRead;
    i2s_read(I2S_NUM_0, sampleBuffer,
             sizeof(sampleBuffer), &bytesRead, portMAX_DELAY);

    // 2. Siapkan data untuk inference
    signal_t signal;
    int err = numpy::signal_from_buffer(sampleBuffer,
              EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

    if (err != 0) {
        Serial.printf("Signal error: %d\n", err);
        return;
    }

    // 3. Jalankan inference (ML prediction)
    ei_impulse_result_t result;
    err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
        Serial.printf("Classifier error: %d\n", err);
        return;
    }

    // 4. Proses hasil
    float maxValue = 0;
    int maxIndex = 0;

    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        if (result.classification[i].value > maxValue) {
            maxValue = result.classification[i].value;
            maxIndex = i;
        }
    }

    String label = ei_classifier_inferencing_categories[maxIndex];
    float confidence = maxValue;

    // Hanya aksi jika confidence > 70%
    if (confidence > 0.7) {
        if (label == "nyala" && !ledState) {
            ledState = true;
            digitalWrite(LED_PIN, HIGH);
            Serial.println("🔊 'nyala' detected → LED ON! 💡");
        }
        else if (label == "mati" && ledState) {
            ledState = false;
            digitalWrite(LED_PIN, LOW);
            Serial.println("🔊 'mati' detected → LED OFF! 🌑");
        }
    }

    // Debug: print semua probabilitas
    Serial.printf("  [");
    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        Serial.printf("%s: %.2f  ",
                       ei_classifier_inferencing_categories[i],
                       result.classification[i].value);
    }
    Serial.printf("] → %s (%.0f%%)\n", label.c_str(), confidence * 100);

    // 5. Tampilkan timing
    Serial.printf("  DSP: %d ms | Classification: %d ms | Total: %d ms\n",
                   result.timing.dsp, result.timing.classification,
                   result.timing.dsp + result.timing.classification);
}
```

---

## 📊 Expected Results

```
Setelah training di Edge Impulse, Anda seharusnya mendapat:

Accuracy: ~85-95% (tergantung kualitas data)

Confusion Matrix (contoh):
            nyala    mati    noise
  nyala     93%      2%      5%
  mati       3%     90%      7%
  noise      1%      1%     98%

Model Size: ~20-50 KB (cukup untuk ESP32!)
Inference time: ~10-50 ms per window

Latency total: ~50-100 ms
(Rekam audio → extract MFCC → inference → output)
→ Hampir real-time! Tidak terasa delay!
```

---

## 📝 Latihan Mandiri

1. **Collect** minimal 30 sample untuk setiap kelas ("nyala", "mati", "noise")
2. **Train** model di Edge Impulse, amati confusion matrix
3. **Test** di Edge Impulse: "Model testing" — cek akurasi dengan data baru
4. **Deploy** ke ESP32 dan coba ucapkan keyword dari berbagai jarak
5. **Tambah keyword** baru: coba "terang" dan "redup" untuk kontrol brightness

---

## 📚 Referensi
- [Edge Impulse Audio Tutorial](https://docs.edgeimpulse.com/docs/tutorials/responding-to-your-voice)
- [MFCC Explained](https://medium.com/analytics-vidhya/understanding-the-mel-spectrogram-fca2aea3d0b)
- [Keyword Spotting](https://arxiv.org/abs/1711.07128)
- [INMP441 I2S Mic Guide](https://randomnerdtutorials.com/esp32-i2s-digital-microphone/)
