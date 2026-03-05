# Modul Pertemuan 12 — IOT Lanjutan
# Computer Vision On-Device — Face Detection & Recognition (ESP-WHO)

---

## 1. Maksud dan Tujuan Materi

### Maksud
ESP32-CAM tidak hanya streaming video — dengan library **ESP-WHO**, ESP32 bisa menjalankan face detection dan face recognition langsung di chip (on-device). Pertemuan ini membahas setup ESP-WHO, enrollment wajah, dan integrasi dengan aktuator (kunci pintu).

### Tujuan Pembelajaran
1. **Menginstal** dan menjalankan ESP-WHO face detection/recognition.
2. **Melakukan** face enrollment via web interface.
3. **Mengintegrasikan** hasil recognition dengan aktuator (relay/servo).

---

## 2. Teori Materi

### 2.1 ESP-WHO

ESP-WHO adalah framework computer vision dari Espressif untuk ESP32 yang mencakup face detection (MTMN algorithm) dan face recognition. Berjalan di ESP32 dengan PSRAM.

**Keterbatasan:**
- Resolusi terbatas (QVGA ~320×240 optimal)
- Latensi ~500ms per frame untuk detection + recognition
- Max ~7 wajah tersimpan
- Akurasi bergantung pada pencahayaan

### 2.2 Alur Kerja

```
  Kamera → Frame → Face Detection (MTMN)
                         │
                    Wajah ditemukan?
                    ├── Ya → Face Recognition
                    │        ├── Dikenal → ✅ Aksi (relay ON)
                    │        └── Tidak dikenal → ❌ Alarm
                    └── Tidak → (skip)
```

---

## 3. Panduan Praktikum

### Praktikum — Flash ESP-WHO Face Recognition

1. **Clone repository:**
```bash
git clone --recursive https://github.com/espressif/esp-who.git
cd esp-who/examples/human_face_recognition/terminal
```

2. **Build dengan ESP-IDF atau PlatformIO:**

**`platformio.ini`:**
```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
board_build.partitions = huge_app.csv
lib_deps =
    espressif/esp32-camera
```

3. **Atau gunakan CameraWebServer yang sudah built-in face detection:**

Menu web ESP32-CAM → aktifkan "Face Detection" toggle → bounding box muncul pada wajah → aktifkan "Face Recognition" → klik "Enroll Face" untuk mendaftarkan wajah.

### Integrasi Aktuator

```cpp
// Pseudo-code integrasi face recognition → relay
// Dalam callback face detection:

if (face_recognized) {
  Serial.println("Wajah dikenal! Membuka kunci...");
  digitalWrite(PIN_RELAY, HIGH);   // Buka kunci
  digitalWrite(PIN_LED_GREEN, HIGH);
  delay(3000);
  digitalWrite(PIN_RELAY, LOW);    // Tutup kembali
  digitalWrite(PIN_LED_GREEN, LOW);
} else if (face_detected_but_unknown) {
  Serial.println("Wajah TIDAK DIKENAL!");
  digitalWrite(PIN_LED_RED, HIGH);
  // Opsional: kirim MQTT alert
  mqtt.publish("iot/security/alert", "{\"event\":\"unknown_face\"}");
  delay(2000);
  digitalWrite(PIN_LED_RED, LOW);
}
```

---

## 4. Referensi

1. **Espressif.** *ESP-WHO.* [https://github.com/espressif/esp-who](https://github.com/espressif/esp-who)
2. **Random Nerd Tutorials.** *ESP32-CAM Face Recognition.* [https://randomnerdtutorials.com/esp32-cam-face-recognition/](https://randomnerdtutorials.com/esp32-cam-face-recognition/)
