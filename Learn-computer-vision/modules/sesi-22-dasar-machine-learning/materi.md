# Sesi 22 — Dasar Machine Learning untuk Pemula

## 🎯 Tujuan Pembelajaran
- Memahami konsep dasar ML: data, label, training, model, inference
- Membedakan supervised vs unsupervised learning
- Membuat classifier pertama dengan scikit-learn
- Memahami overfitting, underfitting, dan evaluasi model
- Mengkonversi model ke TFLite (format untuk mikrokontroler)

---

## 📖 Teori

### Apa itu Machine Learning?

Bayangkan Anda mengajar anak kecil mengenali buah:

```
👶 Belajar Tradisional (Programming):
   "Jika warna merah DAN bulat → Apel"
   "Jika warna kuning DAN panjang → Pisang"
   → Anda tulis SEMUA aturan sendiri

🤖 Machine Learning:
   Tunjukkan 100 foto apel → "Ini apel"
   Tunjukkan 100 foto pisang → "Ini pisang"
   → Komputer BELAJAR SENDIRI aturannya!
```

### Istilah-Istilah Penting

```
┌─────────────────────────────────────────────────────────────┐
│                    KAMUS ML PEMULA                           │
│                                                             │
│  DATASET   = Kumpulan data untuk belajar                    │
│               Contoh: 200 foto buah                         │
│                                                             │
│  LABEL     = Jawaban benar untuk setiap data                │
│               Contoh: foto ini = "apel"                     │
│                                                             │
│  FEATURE   = Ciri-ciri yang digunakan untuk klasifikasi     │
│               Contoh: warna, bentuk, ukuran                 │
│                                                             │
│  MODEL     = "Otak" yang sudah belajar dari data            │
│               Berisi formula/pola yang ditemukan             │
│                                                             │
│  TRAINING  = Proses belajar (kasih data + label)            │
│               Seperti belajar untuk ujian                    │
│                                                             │
│  INFERENCE = Proses menebak data baru                       │
│               Seperti ujian sesungguhnya                     │
│                                                             │
│  EPOCH     = Satu putaran melihat SEMUA data training       │
│               10 epoch = lihat semua data 10 kali           │
│                                                             │
│  ACCURACY  = Persentase tebakan yang benar                  │
│               accuracy 95% = 95 dari 100 benar              │
│                                                             │
│  LOSS      = Ukuran "seberapa salah" model                  │
│               Semakin kecil loss = semakin bagus             │
└─────────────────────────────────────────────────────────────┘
```

### Pipeline ML

```
    1. COLLECT        2. PREPARE       3. TRAIN        4. EVALUATE     5. DEPLOY
   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐
   │ Kumpulkan│    │ Bersihkan│    │ Latih    │    │ Test     │    │ Pasang   │
   │ Data     │───►│ & Label  │───►│ Model    │───►│ Akurasi  │───►│ di Device│
   │          │    │          │    │          │    │          │    │          │
   └──────────┘    └──────────┘    └──────────┘    └──────────┘    └──────────┘
      Sensor,         Split:          Pilih          Confusion        ESP32,
      Kamera,      Train/Test/Val   Algorithm:       Matrix,         Arduino,
      Manual       (80/10/10)       NN, Tree, etc    Accuracy        Raspberry Pi
```

### Supervised vs Unsupervised

```
📚 SUPERVISED LEARNING (Belajar dengan guru)
   Data + Label → Model
   "Ini apel" "Ini pisang" → Model tahu beda apel & pisang
   Contoh: Klasifikasi gambar, keyword spotting

🔍 UNSUPERVISED LEARNING (Belajar sendiri)
   Data TANPA Label → Model cari pola sendiri
   Kumpulan data sensor → Model deteksi "yang aneh"
   Contoh: Anomaly detection, clustering
```

### Overfitting vs Underfitting

```
📊 UNDERFITTING (Kurang belajar)     📊 GOOD FIT (Pas!)          📊 OVERFITTING (Kebanyakan hafal)
   Model terlalu simple               Generalisasi bagus           Hafal data training,
   Akurasi training: 60%              Akurasi training: 93%        gagal di data baru
   Akurasi test: 58%                  Akurasi test: 90%            Akurasi training: 99%
                                                                   Akurasi test: 65%
   Solusi: model lebih complex        ← TARGET INI!                 Solusi: lebih banyak data,
   atau lebih banyak fitur                                          dropout, regularization

Analogi:
   Murid malas, tidak belajar        Murid rajin, paham konsep    Murid hafalkan soal,
   → gagal ujian                     → lulus ujian                 tapi soal beda → gagal
```

---

## 🛠️ Praktik

### 1. Classifier Pertama dengan scikit-learn

```python
"""
sesi22_first_classifier.py
Membuat classifier pertama — Iris flower dataset
Dataset ini berisi 150 bunga iris dengan 4 fitur (petal/sepal length/width)
dan 3 kelas (setosa, versicolor, virginica)

Ini adalah "Hello World" nya Machine Learning!
"""
# Step 1: Import library
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
from sklearn.metrics import accuracy_score, classification_report
import numpy as np

# Step 2: Load dataset
print("=" * 50)
print("STEP 1: Load Dataset")
print("=" * 50)

iris = load_iris()
X = iris.data       # Fitur: 4 angka per sample (panjang/lebar petal & sepal)
y = iris.target      # Label: 0, 1, atau 2 (jenis bunga)

print(f"Total data:  {len(X)} samples")
print(f"Jumlah fitur: {X.shape[1]}")
print(f"Nama fitur:  {iris.feature_names}")
print(f"Kelas:       {iris.target_names}")
print(f"\nContoh data pertama:")
print(f"  Fitur: {X[0]}")
print(f"  Label: {y[0]} ({iris.target_names[y[0]]})")

# Step 3: Split data → Training (80%) dan Testing (20%)
print("\n" + "=" * 50)
print("STEP 2: Split Dataset")
print("=" * 50)

X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42
)

print(f"Data training: {len(X_train)} samples (80%)")
print(f"Data testing:  {len(X_test)} samples (20%)")

# Step 4: Training — ajarkan model!
print("\n" + "=" * 50)
print("STEP 3: Training Model")
print("=" * 50)

model = DecisionTreeClassifier(max_depth=3)  # Model sederhana

print("Training dimulai...")
model.fit(X_train, y_train)  # ← INI PROSES TRAINING!
print("Training selesai! ✅")

# Step 5: Testing — uji model dengan data yang belum pernah dilihat
print("\n" + "=" * 50)
print("STEP 4: Testing & Evaluasi")
print("=" * 50)

y_pred = model.predict(X_test)   # ← INI PROSES INFERENCE!

accuracy = accuracy_score(y_test, y_pred)
print(f"Akurasi: {accuracy:.1%}")  # Persentase benar
print(f"Artinya: {int(accuracy * len(y_test))}/{len(y_test)} tebakan benar!")

print("\nLaporan detail:")
print(classification_report(y_test, y_pred, target_names=iris.target_names))

# Step 6: Coba prediksi data baru
print("=" * 50)
print("STEP 5: Prediksi Data Baru")
print("=" * 50)

# Bunga baru yang belum pernah dilihat
bunga_baru = np.array([[5.1, 3.5, 1.4, 0.2]])  # ukuran petal/sepal
prediksi = model.predict(bunga_baru)
proba = model.predict_proba(bunga_baru)

print(f"Input fitur: {bunga_baru[0]}")
print(f"Prediksi: {iris.target_names[prediksi[0]]}")
print(f"Probabilitas: ", end="")
for nama, prob in zip(iris.target_names, proba[0]):
    print(f"{nama}={prob:.1%} ", end="")
print()
```

### 2. Visualisasi Training Process (Neural Network)

```python
"""
sesi22_training_visualization.py
Visualisasi proses training neural network
Melihat bagaimana loss turun dan accuracy naik selama training
"""
import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import make_moons
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

# Gunakan TensorFlow/Keras
import tensorflow as tf
from tensorflow import keras

# ====== STEP 1: Buat dataset (data bulan sabit — 2 kelas) ======
print("Generating dataset...")
X, y = make_moons(n_samples=500, noise=0.25, random_state=42)
X = StandardScaler().fit_transform(X)

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2)

print(f"Training samples: {len(X_train)}")
print(f"Testing samples:  {len(X_test)}")
print(f"Features per sample: {X.shape[1]}")
print(f"Classes: 2 (0 dan 1)")

# ====== STEP 2: Buat model neural network ======
print("\nBuilding model...")
model = keras.Sequential([
    # Layer 1: input 2 fitur, 16 neuron
    keras.layers.Dense(16, activation='relu', input_shape=(2,)),
    # Layer 2: 8 neuron
    keras.layers.Dense(8, activation='relu'),
    # Layer output: 1 neuron (binary classification)
    keras.layers.Dense(1, activation='sigmoid'),
])

model.compile(
    optimizer='adam',
    loss='binary_crossentropy',
    metrics=['accuracy']
)

# Tampilkan arsitektur model
model.summary()
print(f"\nTotal parameters: {model.count_params()}")
print(f"Model size estimate: ~{model.count_params() * 4 / 1024:.1f} KB (float32)")

# ====== STEP 3: Training ======
print("\n🏋️ Training dimulai...")
history = model.fit(
    X_train, y_train,
    epochs=50,               # 50 putaran belajar
    batch_size=32,            # Proses 32 data sekaligus
    validation_split=0.2,    # 20% data training untuk validasi
    verbose=1                 # Tampilkan progress
)

# ====== STEP 4: Evaluasi ======
test_loss, test_acc = model.evaluate(X_test, y_test, verbose=0)
print(f"\n📊 Test Accuracy: {test_acc:.1%}")
print(f"📊 Test Loss: {test_loss:.4f}")

# ====== STEP 5: Visualisasi ======
fig, axes = plt.subplots(1, 2, figsize=(12, 4))

# Plot Loss
axes[0].plot(history.history['loss'], label='Training Loss', color='blue')
axes[0].plot(history.history['val_loss'], label='Validation Loss', color='red')
axes[0].set_title('Loss per Epoch')
axes[0].set_xlabel('Epoch')
axes[0].set_ylabel('Loss (semakin kecil semakin bagus)')
axes[0].legend()
axes[0].grid(True, alpha=0.3)

# Plot Accuracy
axes[1].plot(history.history['accuracy'], label='Training Acc', color='blue')
axes[1].plot(history.history['val_accuracy'], label='Validation Acc', color='red')
axes[1].set_title('Accuracy per Epoch')
axes[1].set_xlabel('Epoch')
axes[1].set_ylabel('Accuracy (semakin tinggi semakin bagus)')
axes[1].legend()
axes[1].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("training_history.png", dpi=150)
plt.show()

print("\n💡 PERHATIKAN:")
print("  - Loss TURUN seiring epoch → model semakin pinter")
print("  - Accuracy NAIK seiring epoch → tebakan semakin benar")
print("  - Jika val_loss NAIK tapi train_loss TURUN → OVERFITTING!")
```

### 3. Overfitting vs Good Fit — Demonstrasi

```python
"""
sesi22_overfitting_demo.py
Demonstrasi visual perbedaan overfit vs good fit
"""
import numpy as np
import matplotlib.pyplot as plt
from sklearn.datasets import make_moons
from sklearn.tree import DecisionTreeClassifier
from sklearn.model_selection import train_test_split

# Generate data
X, y = make_moons(n_samples=200, noise=0.3, random_state=42)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3)

# === Model 1: UNDERFITTING (terlalu simpel) ===
model_under = DecisionTreeClassifier(max_depth=1)
model_under.fit(X_train, y_train)
acc_train_under = model_under.score(X_train, y_train)
acc_test_under = model_under.score(X_test, y_test)

# === Model 2: GOOD FIT (pas) ===
model_good = DecisionTreeClassifier(max_depth=4)
model_good.fit(X_train, y_train)
acc_train_good = model_good.score(X_train, y_train)
acc_test_good = model_good.score(X_test, y_test)

# === Model 3: OVERFITTING (terlalu complex) ===
model_over = DecisionTreeClassifier(max_depth=20)
model_over.fit(X_train, y_train)
acc_train_over = model_over.score(X_train, y_train)
acc_test_over = model_over.score(X_test, y_test)

# Print results
print("=" * 55)
print(f"{'Model':<20} {'Train Acc':>10} {'Test Acc':>10} {'Gap':>8}")
print("=" * 55)
print(f"{'UNDERFIT (depth=1)':<20} {acc_train_under:>9.1%} {acc_test_under:>9.1%} {abs(acc_train_under-acc_test_under):>7.1%}")
print(f"{'GOOD FIT (depth=4)':<20} {acc_train_good:>9.1%} {acc_test_good:>9.1%} {abs(acc_train_good-acc_test_good):>7.1%}")
print(f"{'OVERFIT (depth=20)':<20} {acc_train_over:>9.1%} {acc_test_over:>9.1%} {abs(acc_train_over-acc_test_over):>7.1%}")
print("=" * 55)
print()
print("💡 Perhatikan:")
print("   UNDERFIT: Train & Test acc rendah → model kurang belajar")
print("   GOOD FIT: Train & Test acc tinggi, gap kecil → IDEAL! ✅")
print("   OVERFIT:  Train acc tinggi, Test acc rendah → hafal, gagal di data baru")
```

### 4. Konversi Model ke TFLite

```python
"""
sesi22_convert_tflite.py
Konversi model Keras ke TensorFlow Lite
Ini adalah format yang akan di-deploy ke ESP32!
"""
import tensorflow as tf
from tensorflow import keras
import numpy as np
from sklearn.datasets import make_moons
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
import os

# ====== Buat dan train model ======
X, y = make_moons(n_samples=500, noise=0.25, random_state=42)
X = StandardScaler().fit_transform(X)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2)

model = keras.Sequential([
    keras.layers.Dense(16, activation='relu', input_shape=(2,)),
    keras.layers.Dense(8, activation='relu'),
    keras.layers.Dense(1, activation='sigmoid'),
])
model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
model.fit(X_train, y_train, epochs=30, verbose=0)

_, test_acc = model.evaluate(X_test, y_test, verbose=0)
print(f"Original model accuracy: {test_acc:.1%}")

# ====== Simpan model Keras ======
model.save("model_keras.h5")
keras_size = os.path.getsize("model_keras.h5")
print(f"Keras model size: {keras_size / 1024:.1f} KB")

# ====== Konversi ke TFLite (Float32) ======
print("\n--- Converting to TFLite (Float32) ---")
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

with open("model_float32.tflite", "wb") as f:
    f.write(tflite_model)

float32_size = len(tflite_model)
print(f"TFLite Float32 size: {float32_size / 1024:.1f} KB")

# ====== Konversi ke TFLite (INT8 Quantized) — untuk ESP32! ======
print("\n--- Converting to TFLite (INT8 Quantized) ---")
converter2 = tf.lite.TFLiteConverter.from_keras_model(model)
converter2.optimizations = [tf.lite.Optimize.DEFAULT]

# Representative dataset untuk quantization
def representative_dataset():
    for i in range(100):
        yield [X_train[i:i+1].astype(np.float32)]

converter2.representative_dataset = representative_dataset
converter2.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter2.inference_input_type = tf.int8
converter2.inference_output_type = tf.int8

tflite_quant = converter2.convert()

with open("model_int8.tflite", "wb") as f:
    f.write(tflite_quant)

int8_size = len(tflite_quant)
print(f"TFLite INT8 size: {int8_size / 1024:.1f} KB")

# ====== Summary ======
print("\n" + "=" * 50)
print("MODEL SIZE COMPARISON")
print("=" * 50)
print(f"Keras (.h5):      {keras_size / 1024:>8.1f} KB")
print(f"TFLite Float32:   {float32_size / 1024:>8.1f} KB  ({float32_size/keras_size*100:.0f}% of Keras)")
print(f"TFLite INT8:      {int8_size / 1024:>8.1f} KB  ({int8_size/keras_size*100:.0f}% of Keras)")
print(f"\nPengurangan: {(1 - int8_size/keras_size)*100:.0f}% lebih kecil!")
print(f"\n✅ Model INT8 ({int8_size / 1024:.1f} KB) bisa masuk ESP32!")
print(f"   ESP32 free RAM: ~200+ KB → CUKUP! 🎉")
```

---

## 📊 Cheat Sheet: ML Pipeline

```
┌─────────────────────────────────────────────────────────────────┐
│                    ML PIPELINE CHEAT SHEET                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  1. KUMPULKAN DATA                                             │
│     └─ Sensor, kamera, download dataset                        │
│                                                                 │
│  2. BERSIHKAN & SPLIT                                          │
│     ├─ Hapus data rusak/noise                                  │
│     └─ Split: Train(80%) / Validation(10%) / Test(10%)         │
│                                                                 │
│  3. PILIH MODEL                                                │
│     ├─ Data kecil → Decision Tree, KNN                         │
│     ├─ Data banyak → Neural Network                            │
│     └─ Gambar → CNN (Convolutional Neural Network)             │
│                                                                 │
│  4. TRAINING                                                   │
│     ├─ Atur: epochs, batch_size, learning_rate                 │
│     └─ Monitor: loss turun? accuracy naik?                     │
│                                                                 │
│  5. EVALUASI                                                   │
│     ├─ Accuracy > 85% → ✅ bagus                              │
│     ├─ Train acc >> Test acc → ⚠️ overfitting                 │
│     └─ Confusion matrix → lihat kelas mana yang salah          │
│                                                                 │
│  6. CONVERT & DEPLOY                                           │
│     ├─ Keras → TFLite → Quantize INT8                          │
│     └─ Embed ke ESP32 firmware                                  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📝 Latihan Mandiri

1. **Jalankan first_classifier.py** dan amati akurasi — ubah `max_depth` dari 1 sampai 10, bagaimana akurasi berubah?
2. **Jalankan training_visualization.py** — apakah ada tanda overfitting di grafik?
3. **Jalankan overfitting_demo.py** — pahami mengapa gap antara train & test accuracy penting
4. **Convert model ke TFLite** — bandingkan ukuran float32 vs INT8
5. **Challenge**: Buat classifier untuk data sensor acak (generate dengan `make_blobs`)

---

## 📚 Referensi
- [scikit-learn Tutorial](https://scikit-learn.org/stable/tutorial/)
- [TensorFlow Keras Guide](https://www.tensorflow.org/tutorials)
- [TFLite Conversion](https://www.tensorflow.org/lite/convert)
- [Quantization Explained](https://www.tensorflow.org/lite/performance/post_training_quantization)
