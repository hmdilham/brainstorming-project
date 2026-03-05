# Modul Pertemuan 11 — IOT Lanjutan
# ESP32-CAM — Setup, Konfigurasi & Video Streaming

---

## 1. Maksud dan Tujuan Materi

### Maksud
ESP32-CAM adalah modul ESP32 dengan kamera OV2640 yang mampu streaming video via WiFi. Pertemuan ini membahas setup hardware, upload firmware CameraWebServer, konfigurasi parameter kamera, dan akses stream MJPEG dari browser dan Python.

### Tujuan Pembelajaran
1. **Mengonfigurasi** ESP32-CAM dengan FTDI adapter untuk upload firmware.
2. **Menjalankan** CameraWebServer untuk live streaming MJPEG.
3. **Mengakses** video stream dari browser dan Python script.
4. **Mengoptimalkan** kualitas vs framerate.

---

## 2. Teori Materi

### 2.1 ESP32-CAM Hardware

| Komponen | Spesifikasi |
|---|---|
| Chip | ESP32-S |
| Kamera | OV2640, 2MP |
| Flash LED | Onboard (GPIO 4) |
| MicroSD Slot | Ya (SPI bus) |
| PSRAM | 4MB (untuk frame buffer besar) |
| USB | **TIDAK ADA** — perlu FTDI adapter untuk upload |

### 2.2 Upload via FTDI

```
  FTDI Adapter           ESP32-CAM
  ┌──────────┐          ┌──────────┐
  │      3.3V├──────────┤ 3.3V     │
  │       GND├──────────┤ GND      │
  │       TXD├──────────┤ UOR (RX) │
  │       RXD├──────────┤ UOT (TX) │
  └──────────┘          │          │
                        │ IO0 ─┐   │  ← Hubungkan IO0 ke GND
                        │ GND ─┘   │    saat upload!
                        └──────────┘
  
  Setelah upload: lepas IO0 dari GND → tekan RESET
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — CameraWebServer

**`platformio.ini`:**
```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
upload_speed = 460800
board_build.partitions = huge_app.csv
```

**`src/main.cpp`:**
```cpp
#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>

// === Pin kamera AI-Thinker ESP32-CAM ===
#define PWDN_GPIO     32
#define RESET_GPIO    -1
#define XCLK_GPIO      0
#define SIOD_GPIO     26
#define SIOC_GPIO     27
#define Y9_GPIO       35
#define Y8_GPIO       34
#define Y7_GPIO       39
#define Y6_GPIO       36
#define Y5_GPIO       21
#define Y4_GPIO       19
#define Y3_GPIO       18
#define Y2_GPIO        5
#define VSYNC_GPIO    25
#define HREF_GPIO     23
#define PCLK_GPIO     22

const char* SSID = "WIFI_ANDA";
const char* PASS = "PASSWORD";

void startCameraServer(); // Didefinisikan di contoh CameraWebServer

void setup() {
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO;
  config.pin_d1       = Y3_GPIO;
  config.pin_d2       = Y4_GPIO;
  config.pin_d3       = Y5_GPIO;
  config.pin_d4       = Y6_GPIO;
  config.pin_d5       = Y7_GPIO;
  config.pin_d6       = Y8_GPIO;
  config.pin_d7       = Y9_GPIO;
  config.pin_xclk     = XCLK_GPIO;
  config.pin_pclk     = PCLK_GPIO;
  config.pin_vsync    = VSYNC_GPIO;
  config.pin_href     = HREF_GPIO;
  config.pin_sccb_sda = SIOD_GPIO;
  config.pin_sccb_scl = SIOC_GPIO;
  config.pin_pwdn     = PWDN_GPIO;
  config.pin_reset    = RESET_GPIO;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;     // 640x480
  config.jpeg_quality = 12;                // 0-63 (lower=better)
  config.fb_count     = 2;                 // Double buffer
  config.grab_mode    = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size   = FRAMESIZE_SVGA;  // 800x600 dengan PSRAM
    config.jpeg_quality = 10;
    config.fb_count     = 2;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return;
  }

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  startCameraServer();

  Serial.printf("\nCamera ready!\n");
  Serial.printf("Stream : http://%s:81/stream\n", WiFi.localIP().toString().c_str());
  Serial.printf("Control: http://%s\n", WiFi.localIP().toString().c_str());
}

void loop() {
  delay(10000);
}
```

> **Note:** Gunakan contoh lengkap CameraWebServer dari Arduino ESP32 Examples yang menyertakan `app_httpd.cpp` untuk web server dan streaming endpoint.

### Praktikum 3.2 — Akses Stream dari Python

```python
# stream_viewer.py
import cv2
import urllib.request
import numpy as np

ESP32_CAM_URL = "http://192.168.1.XXX:81/stream"

stream = urllib.request.urlopen(ESP32_CAM_URL)
buffer = bytes()

while True:
    buffer += stream.read(4096)
    start = buffer.find(b'\xff\xd8')  # JPEG start
    end = buffer.find(b'\xff\xd9')    # JPEG end

    if start != -1 and end != -1:
        jpg = buffer[start:end+2]
        buffer = buffer[end+2:]

        frame = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
        if frame is not None:
            cv2.imshow("ESP32-CAM Stream", frame)

    if cv2.waitKey(1) == 27:  # ESC
        break

cv2.destroyAllWindows()
```

---

## 4. Referensi

1. **Random Nerd Tutorials.** *ESP32-CAM Video Streaming.* [https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/](https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/)
2. **Espressif.** *esp_camera API.* [https://github.com/espressif/esp32-camera](https://github.com/espressif/esp32-camera)
