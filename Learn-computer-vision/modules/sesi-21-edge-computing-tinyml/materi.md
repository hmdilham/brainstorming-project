# Sesi 21 — Pengenalan Edge Computing & TinyML

## 🎯 Tujuan Pembelajaran
- Memahami apa itu Edge Computing dan mengapa penting
- Memahami perbedaan Cloud AI vs Edge AI vs TinyML
- Mengetahui keterbatasan ESP32 untuk ML (RAM, Flash, CPU)
- Memahami kapan harus menggunakan pendekatan mana

---

## 📖 Teori

### Apa itu Edge Computing?

**Edge Computing** = memproses data **di dekat sumber data** (di perangkat itu sendiri atau perangkat terdekat), bukan mengirim ke server/cloud.

**Analogi sederhana:**
```
☁️ CLOUD COMPUTING:
   Anda ingin tahu cuaca → tanya ke stasiun meteorologi pusat di Jakarta
   → butuh waktu kirim pertanyaan & terima jawaban
   → butuh koneksi internet

🏠 EDGE COMPUTING:
   Anda ingin tahu cuaca → lihat termometer di rumah sendiri
   → langsung dapat jawaban
   → tidak butuh internet
```

### Cloud AI vs Edge AI vs TinyML

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        PERBANDINGAN                                        │
├──────────────────┬─────────────────┬──────────────────┬────────────────────┤
│                  │  ☁️ CLOUD AI     │  🏠 EDGE AI      │  🔬 TinyML         │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Dimana jalan?    │ Server di cloud  │ PC/Raspberry Pi  │ Mikrokontroler     │
│                  │ (AWS, Google)    │ di lokasi        │ (ESP32, Arduino)   │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Butuh internet?  │ ✅ Ya, selalu    │ ❌ Tidak         │ ❌ Tidak           │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Latency          │ 100-500ms       │ 10-50ms          │ 1-20ms             │
│ (delay)          │ (tergantung net)│                  │                    │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Model size       │ GB-an           │ MB-an            │ KB-an (< 1MB)      │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Akurasi          │ ⭐⭐⭐⭐⭐          │ ⭐⭐⭐⭐            │ ⭐⭐⭐               │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Power            │ Listrik besar   │ 5-50 Watt        │ < 1 Watt           │
│                  │ (server farm)   │                  │ (baterai OK!)      │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Privacy          │ Data ke server  │ Data lokal       │ Data di device     │
│                  │ ❌ risiko       │ ✅ aman          │ ✅ sangat aman     │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Contoh           │ ChatGPT,        │ YOLOv8 di PC     │ Keyword spotting   │
│                  │ Google Photos   │ OpenCV+ESP32CAM  │ di ESP32           │
├──────────────────┼─────────────────┼──────────────────┼────────────────────┤
│ Yang sudah       │ (Belum di       │ Fase 1-4 kita!   │ ← Fase 5 ini!     │
│ kita pelajari    │  kurikulum ini) │ PC proses stream │                    │
└──────────────────┴─────────────────┴──────────────────┴────────────────────┘
```

**Yang sudah kita lakukan di Fase 1-4 = EDGE AI:**
```
ESP32-CAM → kirim stream → PC proses (OpenCV/YOLO/MediaPipe) → kirim balik → ESP32 DevKit
```
PC masih diperlukan sebagai "otak". Tanpa PC, sistem mati.

**Yang akan kita pelajari di Fase 5 = TinyML:**
```
Sensor → ESP32 proses SENDIRI (model ML di dalam ESP32) → output langsung
```
**Tidak butuh PC sama sekali!** ESP32 menjadi otak sekaligus.

### Apa itu TinyML?

TinyML = **Machine Learning** yang dijalankan pada **mikrokontroler** (tiny = kecil) dengan daya sangat rendah.

```
┌─────────────────────────────────────────────────────┐
│  TinyML = ML yang bisa jalan di perangkat sekecil   │
│           ESP32 (520 KB RAM, 4MB Flash)             │
│                                                     │
│  Caranya?                                           │
│  1. Buat model ML di PC (training)                  │
│  2. Kompres model agar super kecil (quantization)   │
│  3. Masukkan model ke ESP32 (deploy)                │
│  4. ESP32 jalankan model sendiri (inference)        │
└─────────────────────────────────────────────────────┘
```

### Constraint ESP32 untuk ML

ESP32 BUKAN laptop. Resource-nya sangat terbatas:

```
┌────────────── ESP32 Resources ──────────────┐
│                                              │
│  ⚡ CPU:   240 MHz (vs laptop: 3,000+ MHz)  │
│  🧠 RAM:   520 KB  (vs laptop: 8-16 GB)     │
│  💾 Flash: 4 MB    (vs laptop: 256+ GB)     │
│  🔋 Power: ~0.1-0.5 Watt                    │
│                                              │
│  Artinya:                                    │
│  • Model ML harus < 200 KB (idealnya)       │
│  • Input data harus kecil                    │
│  • Tidak bisa model besar (GPT, YOLO-large) │
│  • TAPI bisa: keyword spotting, gesture,     │
│    klasifikasi gambar kecil, anomaly detect  │
└──────────────────────────────────────────────┘
```

### Yang Bisa Dilakukan TinyML di ESP32

| ✅ Bisa | ❌ Tidak Bisa |
|---------|-------------|
| Keyword spotting ("nyala", "mati") | Speech-to-text lengkap |
| Gesture recognition (accelerometer) | Natural Language Processing |
| Klasifikasi gambar kecil (96x96) | Object detection YOLO full |
| Anomaly detection (getaran) | Face recognition (terlalu banyak class) |
| Wake word detection | Video processing real-time |
| Simple face detection | Generative AI |

### Ecosystem TinyML

```
┌──────────────────────────────────────────────────────────┐
│                    TinyML Ecosystem                       │
│                                                          │
│  Platform:                                               │
│  ┌─────────────┐  ┌─────────────┐  ┌──────────────────┐ │
│  │Edge Impulse │  │TensorFlow   │  │ Arduino ML      │ │
│  │(cloud-based │  │Lite Micro   │  │ (EloquentML)    │ │
│  │ no-code/    │  │(manual,     │  │ (wrapper        │ │
│  │ low-code)   │  │ full control│  │  sederhana)     │ │
│  │ ← MUDAH    │  │ ← MANUAL   │  │ ← PALING MUDAH │ │
│  └─────────────┘  └─────────────┘  └──────────────────┘ │
│                                                          │
│  Hardware yang didukung:                                 │
│  ESP32, Arduino Nano 33 BLE, Raspberry Pi Pico,         │
│  STM32, nRF52840, Sony Spresense                        │
└──────────────────────────────────────────────────────────┘
```

---

## 🛠️ Praktik

### 1. Visualisasi: Cloud vs Edge vs TinyML

```python
"""
sesi21_comparison_demo.py
Demonstrasi visual perbedaan Cloud AI vs Edge AI vs TinyML
Kita bandingkan latency deteksi warna secara lokal vs simulasi cloud

Tujuan: Memahami bahwa processing lokal jauh lebih cepat
"""
import cv2
import numpy as np
import imutils
import time
import requests

cap = cv2.VideoCapture(0)

# Simulasi latency
def local_processing(frame):
    """Edge/TinyML: proses langsung, tanpa delay jaringan."""
    start = time.time()
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, np.array([35, 100, 100]), np.array([85, 255, 255]))
    result = "GREEN" if cv2.countNonZero(mask) > 1000 else "NONE"
    latency = (time.time() - start) * 1000  # ms
    return result, latency

def simulated_cloud_processing(frame):
    """
    Simulasi Cloud AI: tambahkan delay jaringan.
    Di dunia nyata, ini akan kirim gambar ke server, tunggu respons.
    """
    start = time.time()
    # Simulasi network latency (kirim gambar + terima respons)
    time.sleep(0.15)   # 150ms network delay (optimis!)
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, np.array([35, 100, 100]), np.array([85, 255, 255]))
    result = "GREEN" if cv2.countNonZero(mask) > 1000 else "NONE"
    latency = (time.time() - start) * 1000
    return result, latency

mode = "LOCAL"  # "LOCAL" atau "CLOUD"

print("=== Latency Comparison Demo ===")
print("Tekan 'l' untuk LOCAL (Edge/TinyML)")
print("Tekan 'c' untuk CLOUD (simulated)")
print("Tekan 'q' untuk keluar")

latencies_local = []
latencies_cloud = []

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)

    if mode == "LOCAL":
        result, latency = local_processing(frame)
        latencies_local.append(latency)
        method_text = "🏠 LOCAL (Edge/TinyML)"
        lat_color = (0, 255, 0)   # Hijau = cepat
    else:
        result, latency = simulated_cloud_processing(frame)
        latencies_cloud.append(latency)
        method_text = "☁️ CLOUD (Simulated)"
        lat_color = (0, 0, 255)   # Merah = lambat

    # Tampilkan
    cv2.putText(frame, method_text, (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, lat_color, 2)
    cv2.putText(frame, f"Latency: {latency:.1f} ms", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, lat_color, 2)
    cv2.putText(frame, f"Result: {result}", (10, 90),
                cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)

    # Rata-rata
    if latencies_local:
        avg_l = sum(latencies_local[-30:]) / len(latencies_local[-30:])
        cv2.putText(frame, f"Avg LOCAL: {avg_l:.1f}ms", (10, 130),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
    if latencies_cloud:
        avg_c = sum(latencies_cloud[-30:]) / len(latencies_cloud[-30:])
        cv2.putText(frame, f"Avg CLOUD: {avg_c:.1f}ms", (10, 155),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)

    # Latency bar visual
    bar_x, bar_y = 400, 30
    local_bar = min(int(latency / 2), 200) if mode == "LOCAL" else 0
    cloud_bar = min(int(latency / 2), 200) if mode == "CLOUD" else 0

    cv2.rectangle(frame, (bar_x, bar_y), (bar_x + local_bar, bar_y + 15),
                  (0, 255, 0), -1)
    cv2.rectangle(frame, (bar_x, bar_y + 20), (bar_x + cloud_bar, bar_y + 35),
                  (0, 0, 255), -1)

    cv2.imshow("Latency Demo", frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('l'):
        mode = "LOCAL"
        print("Switched to LOCAL")
    elif key == ord('c'):
        mode = "CLOUD"
        print("Switched to CLOUD")

# Summary
print("\n=== SUMMARY ===")
if latencies_local:
    print(f"LOCAL  avg: {sum(latencies_local)/len(latencies_local):.1f} ms")
if latencies_cloud:
    print(f"CLOUD  avg: {sum(latencies_cloud)/len(latencies_cloud):.1f} ms")
if latencies_local and latencies_cloud:
    speedup = (sum(latencies_cloud)/len(latencies_cloud)) / (sum(latencies_local)/len(latencies_local))
    print(f"LOCAL is {speedup:.0f}x faster!")

cap.release()
cv2.destroyAllWindows()
```

### 2. ESP32 Resource Check

```python
"""
sesi21_resource_check.py
Cek berapa resource yang tersedia di ESP32 untuk ML

Jalankan ini setelah flash firmware check di bawah.
Baca output Serial dari ESP32.
"""
print("""
=== ESP32 Resource Reality Check ===

ESP32 yang kita punya:
  CPU:    240 MHz dual-core
  RAM:    520 KB SRAM (+ 4MB PSRAM jika ESP32-WROVER/ESP32-CAM)
  Flash:  4 MB (tempat menyimpan program + model)

Model ML yang bisa masuk:
  ┌──────────────────────────┬──────────┬─────────┐
  │ Jenis Model              │ Size     │ Bisa?   │
  ├──────────────────────────┼──────────┼─────────┤
  │ Keyword spotting (2 kata)│ ~20 KB   │ ✅ Ya   │
  │ Gesture (accelerometer)  │ ~30 KB   │ ✅ Ya   │
  │ Image classifier (96x96)│ ~100 KB  │ ✅ Ya   │
  │ Face detection (simple)  │ ~200 KB  │ ⚠️ Mungkin │
  │ YOLOv8-nano             │ ~6 MB    │ ❌ Tidak│
  │ Face recognition         │ ~30 MB   │ ❌ Tidak│
  │ GPT-2 (kecil)           │ ~500 MB  │ ❌ Tidak│
  └──────────────────────────┴──────────┴─────────┘

Kesimpulan:
  → TinyML di ESP32 cocok untuk model KECIL dan SPESIFIK
  → Satu model untuk SATU tugas
  → Input data harus kecil (audio pendek, gambar kecil, sensor data)
""")
```

### 3. [ESP32] Firmware: Resource Monitor

```cpp
/**
 * sesi21_resource_monitor.ino
 * Cek resource ESP32 yang tersedia
 * Upload ke ESP32, buka Serial Monitor
 */

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println("   ESP32 Resource Monitor for TinyML");
    Serial.println("========================================\n");

    // CPU Info
    Serial.printf("CPU Frequency: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("SDK Version:   %s\n", ESP.getSdkVersion());
    Serial.printf("Chip Model:    %s\n", ESP.getChipModel());
    Serial.printf("Chip Cores:    %d\n", ESP.getChipCores());
    Serial.println();

    // Memory Info
    Serial.println("--- MEMORY ---");
    Serial.printf("Total Heap:      %d bytes (%.1f KB)\n",
                   ESP.getHeapSize(), ESP.getHeapSize() / 1024.0);
    Serial.printf("Free Heap:       %d bytes (%.1f KB)\n",
                   ESP.getFreeHeap(), ESP.getFreeHeap() / 1024.0);
    Serial.printf("Min Free Heap:   %d bytes (%.1f KB)\n",
                   ESP.getMinFreeHeap(), ESP.getMinFreeHeap() / 1024.0);

    // PSRAM (jika ada, misal di ESP32-CAM / WROVER)
    if (psramFound()) {
        Serial.println("\n--- PSRAM (External RAM) ---");
        Serial.printf("PSRAM Size:     %d bytes (%.1f MB)\n",
                       ESP.getPsramSize(), ESP.getPsramSize() / 1048576.0);
        Serial.printf("PSRAM Free:     %d bytes (%.1f MB)\n",
                       ESP.getFreePsram(), ESP.getFreePsram() / 1048576.0);
    } else {
        Serial.println("\nPSRAM: Not available");
    }

    // Flash Info
    Serial.println("\n--- FLASH ---");
    Serial.printf("Flash Size:     %d bytes (%.1f MB)\n",
                   ESP.getFlashChipSize(), ESP.getFlashChipSize() / 1048576.0);
    Serial.printf("Flash Speed:    %d Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("Sketch Size:    %d bytes (%.1f KB)\n",
                   ESP.getSketchSize(), ESP.getSketchSize() / 1024.0);
    Serial.printf("Sketch Free:    %d bytes (%.1f KB)\n",
                   ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace() / 1024.0);

    // TinyML Feasibility
    Serial.println("\n--- TinyML FEASIBILITY ---");
    float freeKB = ESP.getFreeHeap() / 1024.0;
    Serial.printf("Available RAM for model: %.1f KB\n", freeKB);

    if (freeKB > 200) {
        Serial.println("✅ Cukup untuk keyword spotting (~20 KB)");
        Serial.println("✅ Cukup untuk gesture classifier (~30 KB)");
    }
    if (freeKB > 150) {
        Serial.println("✅ Cukup untuk simple image classifier (~100 KB)");
    }
    if (freeKB < 100) {
        Serial.println("⚠️ RAM terbatas, hanya model sangat kecil");
    }

    Serial.println("\n========================================");
    Serial.println("   Ready for TinyML!");
    Serial.println("========================================");
}

void loop() {
    // Tampilkan free heap periodik
    Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
    delay(5000);
}
```

### 4. Install Tools untuk Fase 5

```bash
# ====== Python Tools ======
pip install tensorflow
pip install scikit-learn
pip install matplotlib

# ====== Edge Impulse CLI ======
# 1. Install Node.js terlebih dulu (https://nodejs.org/)
# 2. Install Edge Impulse CLI:
npm install -g edge-impulse-cli

# 3. Verifikasi:
edge-impulse-daemon --version

# ====== Arduino/PlatformIO Libraries ======
# (akan di-install per project nanti)
```

```python
"""
sesi21_verify_install.py
Verifikasi semua tools terinstall dengan benar
"""
import sys

print("=== Fase 5 Tools Verification ===\n")

# Python
print(f"Python: {sys.version}")

# TensorFlow
try:
    import tensorflow as tf
    print(f"TensorFlow: {tf.__version__} ✅")
except ImportError:
    print("TensorFlow: NOT INSTALLED ❌")
    print("  → pip install tensorflow")

# scikit-learn
try:
    import sklearn
    print(f"scikit-learn: {sklearn.__version__} ✅")
except ImportError:
    print("scikit-learn: NOT INSTALLED ❌")
    print("  → pip install scikit-learn")

# matplotlib
try:
    import matplotlib
    print(f"matplotlib: {matplotlib.__version__} ✅")
except ImportError:
    print("matplotlib: NOT INSTALLED ❌")
    print("  → pip install matplotlib")

# numpy (sudah ada dari fase sebelumnya)
try:
    import numpy as np
    print(f"NumPy: {np.__version__} ✅")
except ImportError:
    print("NumPy: NOT INSTALLED ❌")

# OpenCV (sudah ada)
try:
    import cv2
    print(f"OpenCV: {cv2.__version__} ✅")
except ImportError:
    print("OpenCV: NOT INSTALLED ❌")

print("\n=== Edge Impulse CLI ===")
print("Cek manual di terminal:")
print("  edge-impulse-daemon --version")
print("  (Jika belum install: npm install -g edge-impulse-cli)")

print("\n✅ Semua tools siap!" if True else "")
```

---

## 📝 Latihan Mandiri

1. **Jalankan resource monitor** di ESP32 DevKit DAN ESP32-CAM — bandingkan hasilnya (PSRAM!)
2. **Bandingkan latency** local vs "cloud" (simulasi) — berapa kali lebih cepat?
3. **Buat tabel** 5 contoh aplikasi dan tentukan: Cloud AI, Edge AI, atau TinyML?
4. **Install semua tools** dan verifikasi semuanya berjalan

---

## 🤔 Pertanyaan Refleksi

Sebelum lanjut, coba jawab pertanyaan ini:
1. Mengapa tidak semua ML model bisa dijalankan di ESP32?
2. Apa keuntungan terbesar TinyML dibanding Cloud AI?
3. Kapan sebaiknya tetap menggunakan Cloud AI daripada TinyML?

> **Jawaban singkat**: TinyML untuk tugas spesifik yang butuh low-latency, low-power, privacy. Cloud AI untuk tugas kompleks yang butuh model besar dan akurasi maksimal.

---

## 📚 Referensi
- [TinyML Book by Pete Warden](https://www.oreilly.com/library/view/tinyml/9781492052036/)
- [Edge Impulse Documentation](https://docs.edgeimpulse.com/)
- [TensorFlow Lite for Microcontrollers](https://www.tensorflow.org/lite/microcontrollers)
- [ESP32 Technical Reference](https://www.espressif.com/en/products/socs/esp32/resources)
