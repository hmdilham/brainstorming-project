# Sesi 5 — Komunikasi ESP32-CAM ↔ ESP32 DevKit ↔ PC

## 🎯 Tujuan Pembelajaran
- Menghubungkan PC dengan ESP32 DevKit via Serial dan WiFi
- Mengirim perintah dari Python ke ESP32 untuk mengontrol aktuator
- Membangun pipeline end-to-end: Kamera → PC (CV) → ESP32 (Aktuator)
- Mini Project: ON/OFF LED berdasarkan deteksi warna

---

## 📖 Teori

### Arsitektur Komunikasi
```
  ┌──────────┐       ┌──────────────────┐       ┌──────────────┐
  │  Kamera  │       │   PC (Python)    │       │  ESP32 DevKit│
  │ (Webcam/ │──────►│ CV Processing    │──────►│  GPIO Output │
  │ ESP32CAM)│ Video │ Deteksi → Kirim  │Serial │  LED/Relay   │
  └──────────┘       │    Perintah      │ WiFi  │  Servo/Motor │
                     └──────────────────┘       └──────────────┘
```

### Metode Komunikasi
| Metode | Kelebihan | Kekurangan |
|--------|-----------|------------|
| **Serial (UART)** | Simpel, reliable, low latency | Butuh kabel USB |
| **WiFi HTTP** | Wireless, jarak jauh | Latency lebih tinggi |
| **WiFi WebSocket** | Wireless, real-time, bidirectional | Lebih kompleks |

---

## 🛠️ Praktik

### 1. [ESP32] Firmware: Menerima Perintah Serial

```cpp
/**
 * sesi05_serial_receiver.ino
 * ESP32 DevKit menerima perintah via Serial dan mengontrol LED/Relay
 */

#define LED_PIN     2     // Built-in LED
#define RELAY_PIN   4     // Relay module
#define BUZZER_PIN  5     // Buzzer

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("ESP32 Ready - Waiting for commands...");
    Serial.println("Commands: LED_ON, LED_OFF, RELAY_ON, RELAY_OFF, BUZZ");
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toUpperCase();

        Serial.print("Received: ");
        Serial.println(command);

        if (command == "LED_ON") {
            digitalWrite(LED_PIN, HIGH);
            Serial.println(">> LED ON");
        }
        else if (command == "LED_OFF") {
            digitalWrite(LED_PIN, LOW);
            Serial.println(">> LED OFF");
        }
        else if (command == "RELAY_ON") {
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println(">> RELAY ON");
        }
        else if (command == "RELAY_OFF") {
            digitalWrite(RELAY_PIN, LOW);
            Serial.println(">> RELAY OFF");
        }
        else if (command == "BUZZ") {
            digitalWrite(BUZZER_PIN, HIGH);
            delay(200);
            digitalWrite(BUZZER_PIN, LOW);
            Serial.println(">> BUZZ!");
        }
        else {
            Serial.println(">> Unknown command");
        }
    }
}
```

### 2. [Python] Kirim Perintah via Serial

```python
"""
sesi05_serial_sender.py
Mengirim perintah ke ESP32 DevKit via Serial
"""
import serial
import time

# Ganti port sesuai OS:
# Linux  : /dev/ttyUSB0 atau /dev/ttyACM0
# Windows: COM3, COM4, dll
# Mac    : /dev/cu.usbserial-xxxx
PORT = "/dev/ttyUSB0"
BAUD = 115200

try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
    time.sleep(2)  # Tunggu ESP32 restart
    print(f"Connected to {PORT}")
except Exception as e:
    print(f"Error connecting: {e}")
    exit()

# Test kirim perintah
commands = ["LED_ON", "LED_OFF", "RELAY_ON", "RELAY_OFF", "BUZZ"]

for cmd in commands:
    print(f"\nSending: {cmd}")
    ser.write(f"{cmd}\n".encode())
    time.sleep(0.5)

    # Baca respons
    while ser.in_waiting:
        response = ser.readline().decode().strip()
        print(f"  ESP32: {response}")

    time.sleep(1)

ser.close()
print("\nDone!")
```

### 3. [ESP32] Firmware: Menerima Perintah via WiFi (HTTP)

```cpp
/**
 * sesi05_wifi_receiver.ino
 * ESP32 DevKit menerima perintah via HTTP untuk kontrol aktuator
 */
#include <WiFi.h>
#include <WebServer.h>

const char *ssid = "SSID_ANDA";
const char *password = "PASSWORD_ANDA";

#define LED_PIN     2
#define RELAY_PIN   4

WebServer server(80);

void handleRoot() {
    String html = "<html><body>";
    html += "<h1>ESP32 Control Panel</h1>";
    html += "<p><a href='/led/on'><button>LED ON</button></a>";
    html += " <a href='/led/off'><button>LED OFF</button></a></p>";
    html += "<p><a href='/relay/on'><button>RELAY ON</button></a>";
    html += " <a href='/relay/off'><button>RELAY OFF</button></a></p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleLedOn() {
    digitalWrite(LED_PIN, HIGH);
    server.send(200, "text/plain", "LED ON");
}

void handleLedOff() {
    digitalWrite(LED_PIN, LOW);
    server.send(200, "text/plain", "LED OFF");
}

void handleRelayOn() {
    digitalWrite(RELAY_PIN, HIGH);
    server.send(200, "text/plain", "RELAY ON");
}

void handleRelayOff() {
    digitalWrite(RELAY_PIN, LOW);
    server.send(200, "text/plain", "RELAY OFF");
}

// API endpoint untuk perintah dari Python CV
void handleCommand() {
    if (server.hasArg("action")) {
        String action = server.arg("action");
        action.toUpperCase();

        if (action == "LED_ON") {
            digitalWrite(LED_PIN, HIGH);
        } else if (action == "LED_OFF") {
            digitalWrite(LED_PIN, LOW);
        } else if (action == "RELAY_ON") {
            digitalWrite(RELAY_PIN, HIGH);
        } else if (action == "RELAY_OFF") {
            digitalWrite(RELAY_PIN, LOW);
        }

        server.send(200, "text/plain", "OK: " + action);
    } else {
        server.send(400, "text/plain", "Missing 'action' parameter");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);

    WiFi.begin(ssid, password);
    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/led/on", handleLedOn);
    server.on("/led/off", handleLedOff);
    server.on("/relay/on", handleRelayOn);
    server.on("/relay/off", handleRelayOff);
    server.on("/cmd", handleCommand);

    server.begin();
    Serial.println("HTTP Server started");
    Serial.printf("Control: http://%s/\n", WiFi.localIP().toString().c_str());
    Serial.printf("API:     http://%s/cmd?action=LED_ON\n",
                   WiFi.localIP().toString().c_str());
}

void loop() {
    server.handleClient();
}
```

### 4. [Python] Kirim Perintah via WiFi (HTTP)

```python
"""
sesi05_wifi_sender.py
Mengirim perintah ke ESP32 DevKit via HTTP
"""
import requests
import time

ESP32_IP = "192.168.1.101"  # IP ESP32 DevKit
BASE_URL = f"http://{ESP32_IP}"


def send_command(action):
    """Kirim perintah ke ESP32 via HTTP."""
    try:
        r = requests.get(f"{BASE_URL}/cmd", params={"action": action}, timeout=2)
        print(f"  [{r.status_code}] {r.text}")
        return r.status_code == 200
    except Exception as e:
        print(f"  ERROR: {e}")
        return False


# Test
print("=== Test WiFi Commands ===")
actions = ["LED_ON", "LED_OFF", "RELAY_ON", "RELAY_OFF"]

for action in actions:
    print(f"\nSending: {action}")
    send_command(action)
    time.sleep(1)

print("\nDone!")
```

### 5. [Python] 🔨 Mini Project: Color Detection → Kontrol LED

```python
"""
sesi05_color_control.py
🔨 MINI PROJECT: Deteksi warna dari kamera → kontrol LED/Relay ESP32

Pipeline:
  Kamera (webcam/ESP32-CAM) → Python (deteksi warna) → ESP32 DevKit (LED)

- Terdeteksi warna HIJAU → LED ON
- Tidak terdeteksi      → LED OFF
"""
import cv2
import numpy as np
import imutils
import serial       # Untuk Serial
# import requests   # Untuk WiFi (uncomment jika pakai WiFi)
import time

# ====== PILIH MODE KOMUNIKASI ======
MODE = "SERIAL"   # "SERIAL" atau "WIFI"

# --- Serial Config ---
SERIAL_PORT = "/dev/ttyUSB0"
SERIAL_BAUD = 115200

# --- WiFi Config ---
ESP32_IP = "192.168.1.101"

# ====== PILIH SUMBER KAMERA ======
# Webcam:
cap = cv2.VideoCapture(0)
# ESP32-CAM:
# cap = cv2.VideoCapture("http://192.168.1.100:81/stream")


# ====== Setup Komunikasi ======
ser = None
if MODE == "SERIAL":
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1)
        time.sleep(2)
        print(f"Serial connected: {SERIAL_PORT}")
    except:
        print("Serial connection failed! Running in demo mode.")
        MODE = "DEMO"


def send_to_esp32(command):
    """Kirim perintah ke ESP32."""
    if MODE == "SERIAL" and ser:
        ser.write(f"{command}\n".encode())
    elif MODE == "WIFI":
        try:
            import requests
            requests.get(f"http://{ESP32_IP}/cmd",
                         params={"action": command}, timeout=1)
        except:
            pass
    print(f"  >> {command}")


# ====== HSV Range untuk warna hijau ======
GREEN_LOWER = np.array([35, 100, 100])
GREEN_UPPER = np.array([85, 255, 255])

MIN_AREA = 2000  # Area minimal untuk deteksi valid
last_state = None
debounce_count = 0
DEBOUNCE_THRESHOLD = 5  # Harus terdeteksi 5 frame berturut-turut

print("\n=== Color Detection → LED Control ===")
print("Tunjukkan objek HIJAU ke kamera untuk menyalakan LED")
print("Tekan 'q' untuk keluar\n")

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = imutils.resize(frame, width=640)
    frame = cv2.flip(frame, 1)  # Mirror

    # Konversi ke HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    hsv = cv2.GaussianBlur(hsv, (11, 11), 0)

    # Buat mask warna hijau
    mask = cv2.inRange(hsv, GREEN_LOWER, GREEN_UPPER)
    mask = cv2.erode(mask, None, iterations=2)
    mask = cv2.dilate(mask, None, iterations=2)

    # Cari contour
    contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL,
                                    cv2.CHAIN_APPROX_SIMPLE)

    detected = False
    for cnt in contours:
        area = cv2.contourArea(cnt)
        if area > MIN_AREA:
            detected = True
            x, y, w, h = cv2.boundingRect(cnt)
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.putText(frame, f"GREEN ({int(area)})", (x, y - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)

    # Debounce logic
    new_state = "ON" if detected else "OFF"
    if new_state != last_state:
        debounce_count += 1
        if debounce_count >= DEBOUNCE_THRESHOLD:
            last_state = new_state
            debounce_count = 0
            send_to_esp32(f"LED_{new_state}")
    else:
        debounce_count = 0

    # Status overlay
    status = "DETECTED" if detected else "NOT DETECTED"
    color = (0, 255, 0) if detected else (0, 0, 255)
    cv2.putText(frame, f"Green: {status}", (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
    cv2.putText(frame, f"LED: {last_state or 'OFF'}", (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 0), 2)

    cv2.imshow("Color Detection → LED", frame)
    cv2.imshow("Mask", mask)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Cleanup
send_to_esp32("LED_OFF")
send_to_esp32("RELAY_OFF")
cap.release()
if ser:
    ser.close()
cv2.destroyAllWindows()
```

### 6. Helper: Modul Komunikasi ESP32

```python
"""
utils/esp32_comm.py
Utility class untuk komunikasi dengan ESP32 DevKit
Simpan file ini, akan digunakan di sesi-sesi berikutnya
"""
import serial
import requests
import time


class ESP32Serial:
    """Komunikasi dengan ESP32 via Serial UART."""

    def __init__(self, port="/dev/ttyUSB0", baud=115200):
        self.ser = serial.Serial(port, baud, timeout=1)
        time.sleep(2)

    def send(self, command):
        self.ser.write(f"{command}\n".encode())

    def read(self):
        if self.ser.in_waiting:
            return self.ser.readline().decode().strip()
        return None

    def close(self):
        self.ser.close()


class ESP32WiFi:
    """Komunikasi dengan ESP32 via WiFi HTTP."""

    def __init__(self, ip="192.168.1.101"):
        self.base_url = f"http://{ip}"

    def send(self, command):
        try:
            requests.get(f"{self.base_url}/cmd",
                         params={"action": command}, timeout=2)
        except Exception as e:
            print(f"WiFi send error: {e}")

    def close(self):
        pass


# ============================================
# Factory function
# ============================================
def connect_esp32(method="serial", **kwargs):
    """
    Buat koneksi ke ESP32.

    Args:
        method: "serial" atau "wifi"
        **kwargs: port, baud (serial) atau ip (wifi)
    """
    if method == "serial":
        return ESP32Serial(
            port=kwargs.get("port", "/dev/ttyUSB0"),
            baud=kwargs.get("baud", 115200)
        )
    elif method == "wifi":
        return ESP32WiFi(ip=kwargs.get("ip", "192.168.1.101"))
    else:
        raise ValueError(f"Unknown method: {method}")


# Contoh penggunaan:
if __name__ == "__main__":
    # esp = connect_esp32("serial", port="/dev/ttyUSB0")
    # esp = connect_esp32("wifi", ip="192.168.1.101")

    esp = connect_esp32("serial")
    esp.send("LED_ON")
    time.sleep(1)
    esp.send("LED_OFF")
    esp.close()
```

---

## 📝 Latihan Mandiri

1. **Serial**: Kirim perintah `LED_ON` dan `LED_OFF` dari Python dan verifikasi LED menyala/mati
2. **WiFi**: Akses control panel ESP32 dari browser dan kontrol LED
3. **Mini Project**: Modifikasi color detection agar bisa mendeteksi 2 warna (hijau=ON, merah=OFF)
4. **Challenge**: Buat program yang menghitung jumlah objek berwarna dan tampilkan di OLED ESP32

---

## 📚 Referensi
- [PySerial Documentation](https://pyserial.readthedocs.io/)
- [ESP32 WebServer](https://randomnerdtutorials.com/esp32-web-server-arduino-ide/)
- [Python Requests Library](https://docs.python-requests.org/)
