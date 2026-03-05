# Sesi 4 — Setup ESP32-CAM & Video Streaming

## 🎯 Tujuan Pembelajaran
- Memahami arsitektur hardware ESP32-CAM (AI-Thinker)
- Flash firmware CameraWebServer menggunakan PlatformIO
- Mengakses video stream dari browser dan Python
- Mengatur resolusi, kualitas, dan FPS stream

---

## 📖 Teori

### Arsitektur ESP32-CAM
```
┌─────────────────────────────────────────┐
│              ESP32-CAM                  │
│  ┌──────────┐        ┌──────────────┐  │
│  │ ESP32-S  │◄──────►│  OV2640 Cam  │  │
│  │ (240MHz) │  SCCB  │  2MP sensor  │  │
│  │ WiFi+BT  │        └──────────────┘  │
│  │ 520KB RAM│        ┌──────────────┐  │
│  │ 4MB Flash│◄──────►│  MicroSD     │  │
│  └──────────┘  SPI   │  (opsional)  │  │
│                       └──────────────┘  │
│  Flash LED (GPIO 4)                     │
│  Reset Button                           │
└─────────────────────────────────────────┘
```

### Koneksi Flash (USB-TTL ↔ ESP32-CAM)
| USB-TTL | ESP32-CAM |
|---------|-----------|
| TX | U0R (RX) |
| RX | U0T (TX) |
| GND | GND |
| 5V | 5V |
| — | IO0 → GND (mode flash) |

> ⚠️ **Penting**: Hubungkan IO0 ke GND saat flash firmware, lepas setelah flash selesai, lalu tekan Reset.

### Resolusi vs FPS
| Resolusi | Ukuran | ~FPS (WiFi) |
|----------|--------|:-----------:|
| QQVGA | 160x120 | 25-30 |
| QVGA | 320x240 | 15-20 |
| VGA | 640x480 | 8-12 |
| SVGA | 800x600 | 5-8 |
| XGA | 1024x768 | 3-5 |
| UXGA | 1600x1200 | 1-2 |

> 💡 **Untuk CV processing**: gunakan **QVGA (320x240)** atau **VGA (640x480)** — balance antara detail dan kecepatan.

---

## 🛠️ Praktik

### 1. Setup Project PlatformIO

```ini
; platformio.ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
monitor_speed = 115200
upload_speed = 921600

; Library tambahan jika dibutuhkan
lib_deps =

; Partition scheme untuk kamera
board_build.partitions = huge_app.csv
```

### 2. Firmware CameraWebServer

```cpp
/**
 * sesi04_camera_webserver.ino
 * Firmware untuk ESP32-CAM - Video streaming via WiFi
 */
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

// === WiFi Config ===
const char *ssid = "SSID_ANDA";
const char *password = "PASSWORD_ANDA";

// === Pin kamera (AI-Thinker ESP32-CAM) ===
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

#define FLASH_LED_PIN      4

// HTTP Server
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

// === Inisialisasi Kamera ===
bool initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Resolusi - pilih sesuai kebutuhan
    if (psramFound()) {
        config.frame_size   = FRAMESIZE_VGA;    // 640x480
        config.jpeg_quality = 12;                // 0-63 (lower = better)
        config.fb_count     = 2;
    } else {
        config.frame_size   = FRAMESIZE_QVGA;   // 320x240
        config.jpeg_quality = 15;
        config.fb_count     = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return false;
    }

    // Pengaturan sensor tambahan
    sensor_t *s = esp_camera_sensor_get();
    s->set_brightness(s, 1);    // -2 to 2
    s->set_contrast(s, 1);      // -2 to 2
    s->set_hmirror(s, 0);       // 0 = normal, 1 = mirror
    s->set_vflip(s, 0);         // 0 = normal, 1 = flip

    return true;
}

// === MJPEG Stream Handler ===
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY =
    "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

esp_err_t stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;

    while (true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
            break;
        }

        size_t hlen = snprintf(part_buf, 64, _STREAM_PART, fb->len);
        res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY,
                                     strlen(_STREAM_BOUNDARY));
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, part_buf, hlen);
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);

        esp_camera_fb_return(fb);

        if (res != ESP_OK) break;
    }
    return res;
}

// === Capture Single Frame Handler ===
esp_err_t capture_handler(httpd_req_t *req) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    return res;
}

// === Start HTTP Server ===
void startServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    // Server untuk capture (port 80)
    httpd_uri_t capture_uri = {
        .uri       = "/capture",
        .method    = HTTP_GET,
        .handler   = capture_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }

    // Server untuk stream (port 81)
    config.server_port = 81;
    config.ctrl_port += 1;

    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}

void setup() {
    Serial.begin(115200);

    // Flash LED (off)
    pinMode(FLASH_LED_PIN, OUTPUT);
    digitalWrite(FLASH_LED_PIN, LOW);

    // Init kamera
    if (!initCamera()) {
        Serial.println("Camera init FAILED!");
        return;
    }
    Serial.println("Camera OK");

    // Konek WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start server
    startServer();

    Serial.println("=================================");
    Serial.printf("Stream:  http://%s:81/stream\n",
                   WiFi.localIP().toString().c_str());
    Serial.printf("Capture: http://%s/capture\n",
                   WiFi.localIP().toString().c_str());
    Serial.println("=================================");
}

void loop() {
    delay(10000);
}
```

### 3. [Python] Membaca Stream ESP32-CAM

```python
"""
sesi04_read_stream.py
Membaca dan memproses stream dari ESP32-CAM
"""
import cv2
import imutils
import time

# ===== PILIH SUMBER KAMERA =====
# Webcam:
# cap = cv2.VideoCapture(0)

# ESP32-CAM:
ESP32_IP = "192.168.1.100"  # Ganti dengan IP ESP32-CAM Anda
cap = cv2.VideoCapture(f"http://{ESP32_IP}:81/stream")

if not cap.isOpened():
    print("ERROR: Tidak bisa membuka kamera!")
    print("Cek koneksi WiFi dan IP ESP32-CAM")
    exit()

fps_counter = 0
fps_timer = time.time()
fps = 0

while True:
    ret, frame = cap.read()
    if not ret:
        print("Frame lost, mencoba reconnect...")
        time.sleep(1)
        cap = cv2.VideoCapture(f"http://{ESP32_IP}:81/stream")
        continue

    frame = imutils.resize(frame, width=640)

    # Hitung FPS
    fps_counter += 1
    elapsed = time.time() - fps_timer
    if elapsed >= 1.0:
        fps = fps_counter / elapsed
        fps_counter = 0
        fps_timer = time.time()

    # Overlay info
    h, w = frame.shape[:2]
    cv2.putText(frame, f"FPS: {fps:.1f} | Size: {w}x{h}",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # Terapkan beberapa operasi dari sesi sebelumnya
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, 50, 150)

    cv2.imshow("ESP32-CAM Stream", frame)
    cv2.imshow("Edges", edges)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('s'):
        cv2.imwrite(f"capture_{int(time.time())}.jpg", frame)
        print("Screenshot saved!")

cap.release()
cv2.destroyAllWindows()
```

### 4. [Python] Kontrol Resolusi via HTTP

```python
"""
sesi04_control_resolution.py
Mengubah resolusi ESP32-CAM via HTTP request
"""
import requests
import cv2
import imutils
import time

ESP32_IP = "192.168.1.100"

# Resolusi yang tersedia
RESOLUTIONS = {
    "QQVGA": 0,    # 160x120
    "QVGA":  4,    # 320x240
    "CIF":   5,    # 400x296
    "VGA":   6,    # 640x480
    "SVGA":  7,    # 800x600
    "XGA":   8,    # 1024x768
    "SXGA":  9,    # 1280x1024
    "UXGA": 10,    # 1600x1200
}


def set_resolution(res_name):
    """Ubah resolusi kamera via HTTP."""
    if res_name not in RESOLUTIONS:
        print(f"Resolusi tidak valid: {res_name}")
        return False

    val = RESOLUTIONS[res_name]
    url = f"http://{ESP32_IP}/control?var=framesize&val={val}"
    try:
        r = requests.get(url, timeout=5)
        print(f"Resolusi diubah ke {res_name}: {r.status_code}")
        return r.status_code == 200
    except Exception as e:
        print(f"Error: {e}")
        return False


def set_quality(quality):
    """Ubah kualitas JPEG (4-63, lower = better)."""
    url = f"http://{ESP32_IP}/control?var=quality&val={quality}"
    try:
        r = requests.get(url, timeout=5)
        return r.status_code == 200
    except:
        return False


# Set resolusi optimal untuk CV
set_resolution("VGA")
set_quality(12)

# Buka stream
cap = cv2.VideoCapture(f"http://{ESP32_IP}:81/stream")

print("Tekan 1-4 untuk ganti resolusi:")
print("  1 = QQVGA (160x120)")
print("  2 = QVGA  (320x240)")
print("  3 = VGA   (640x480)")
print("  4 = SVGA  (800x600)")

while True:
    ret, frame = cap.read()
    if not ret:
        continue

    frame = imutils.resize(frame, width=640)

    h_orig, w_orig = frame.shape[:2]
    cv2.putText(frame, f"Original: {w_orig}x{h_orig}",
                (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    cv2.imshow("Stream", frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break
    elif key == ord('1'):
        set_resolution("QQVGA")
    elif key == ord('2'):
        set_resolution("QVGA")
    elif key == ord('3'):
        set_resolution("VGA")
    elif key == ord('4'):
        set_resolution("SVGA")

cap.release()
cv2.destroyAllWindows()
```

---

## ⚡ Troubleshooting

| Masalah | Solusi |
|---------|--------|
| `Camera init failed 0x20001` | Cek koneksi kabel, pastikan 5V stabil |
| Stream lag / putus-putus | Turunkan resolusi, dekatkan ke router WiFi |
| Brownout detector triggered | Power supply kurang, gunakan 5V ≥ 1A |
| Upload gagal | Pastikan IO0 terhubung ke GND saat flash |
| IP tidak muncul | Cek SSID/password, pastikan 2.4GHz WiFi |

---

## 📝 Latihan Mandiri

1. **Flash firmware** ke ESP32-CAM dan akses stream dari browser
2. **Baca stream** dari Python dan terapkan edge detection dari Sesi 2
3. **Buat program** yang capture gambar dari ESP32-CAM setiap 5 detik dan simpan ke folder
4. **Eksperimen** dengan berbagai resolusi dan catat FPS yang didapat

---

## 📚 Referensi
- [ESP32-CAM Getting Started](https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/)
- [ESP-IDF Camera Driver](https://github.com/espressif/esp32-camera)
- [OV2640 Datasheet](https://www.uctronics.com/download/cam_module/OV2640DS.pdf)
