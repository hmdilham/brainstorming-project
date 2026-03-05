# Modul Pertemuan 10 — IOT Dasar
# Komunikasi WiFi ESP32 — STA, AP, NTP

---

## 1. Maksud dan Tujuan Materi

### Maksud
WiFi adalah tulang punggung konektivitas IoT. ESP32 memiliki WiFi 802.11 b/g/n built-in yang memungkinkan perangkat terhubung ke jaringan lokal maupun internet. Pertemuan ini membahas tiga mode WiFi: **Station (STA)**, **Access Point (AP)**, dan **STA+AP**, serta sinkronisasi waktu menggunakan **NTP (Network Time Protocol)**.

### Tujuan Pembelajaran
1. **Menghubungkan** ESP32 ke jaringan WiFi (mode STA) dengan robust reconnect.
2. **Membuat** hotspot WiFi dari ESP32 (mode AP).
3. **Mensinkronkan** waktu real-time dari NTP server.
4. **Menampilkan** informasi jaringan dan jam pada OLED display.

---

## 2. Teori Materi

### 2.1 Mode WiFi ESP32

| Mode | Fungsi | Use Case |
|---|---|---|
| **STA (Station)** | ESP32 terhubung ke router yang ada | Koneksi internet, cloud |
| **AP (Access Point)** | ESP32 membuat hotspot sendiri | Konfigurasi awal, direct control |
| **STA+AP** | Keduanya bersamaan | Gateway: terhubung ke router + client terhubung ke ESP32 |

### 2.2 NTP (Network Time Protocol)

NTP menyinkronize jam perangkat dengan server waktu di internet. ESP32 tidak punya battery-backed RTC, jadi setiap boot, jam dimulai dari 0. NTP memperbaiki ini.

**Server NTP populer:**
- `pool.ntp.org` (global, load-balanced)
- `id.pool.ntp.org` (Indonesia)
- `time.google.com` (Google)

```cpp
#include <time.h>

configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
// WIB (UTC+7): gmtOffset = 7 * 3600 = 25200
```

### 2.3 WiFi Events & Reconnect

```cpp
WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("WiFi terhubung!"); break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi terputus! Reconnecting...");
      WiFi.reconnect(); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str()); break;
  }
});
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — WiFi STA + NTP Clock + OLED

**`platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.9
```

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : WiFi + NTP Clock + OLED
 * DESKRIPSI: Terhubung ke WiFi, sinkronisasi jam via NTP,
 *            menampilkan jam real-time, tanggal, dan info WiFi di OLED.
 *
 * RANGKAIAN:
 *   GPIO 21 → OLED SDA
 *   GPIO 22 → OLED SCL
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

// === KONFIGURASI WiFi ===
const char* WIFI_SSID = "NAMA_WIFI_ANDA";
const char* WIFI_PASS = "PASSWORD_ANDA";

// === KONFIGURASI NTP ===
const char* NTP_SERVER = "pool.ntp.org";
const long  GMT_OFFSET = 25200;  // WIB = UTC+7 = 7*3600
const int   DST_OFFSET = 0;     // Indonesia tidak pakai DST

bool wifiConnected = false;
bool ntpSynced = false;

void connectWiFi() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.setCursor(0, 15);
  display.println(WIFI_SSID);
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    display.setCursor(attempts % 21 * 6, 30);
    display.print(".");
    display.display();
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.printf("\nWiFi OK! IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  } else {
    Serial.println("\nWiFi GAGAL!");
  }
}

void syncNTP() {
  configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);
  Serial.print("Sinkronisasi NTP");

  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 10) {
    Serial.print(".");
    delay(500);
    retry++;
  }

  if (getLocalTime(&timeinfo)) {
    ntpSynced = true;
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("\nNTP OK: %s WIB\n", buf);
  } else {
    Serial.println("\nNTP GAGAL!");
  }
}

void updateOLED() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  struct tm ti;
  bool timeOK = getLocalTime(&ti);

  // Jam besar
  if (timeOK) {
    display.setTextSize(3);
    display.setCursor(8, 0);
    display.printf("%02d:%02d", ti.tm_hour, ti.tm_min);

    // Detik kecil
    display.setTextSize(2);
    display.setCursor(100, 5);
    display.printf("%02d", ti.tm_sec);

    // Tanggal
    display.setTextSize(1);
    display.setCursor(0, 28);
    const char* hari[] = {"Min","Sen","Sel","Rab","Kam","Jum","Sab"};
    const char* bulan[] = {"Jan","Feb","Mar","Apr","Mei","Jun",
                           "Jul","Agu","Sep","Okt","Nov","Des"};
    display.printf("%s, %d %s %d", hari[ti.tm_wday], ti.tm_mday,
                   bulan[ti.tm_mon], ti.tm_year + 1900);
  } else {
    display.setTextSize(2);
    display.setCursor(5, 5);
    display.println("--:--:--");
  }

  display.drawLine(0, 38, 128, 38, SSD1306_WHITE);

  // Info WiFi
  display.setTextSize(1);
  display.setCursor(0, 42);
  if (wifiConnected) {
    display.printf("WiFi: %s", WiFi.localIP().toString().c_str());
    display.setCursor(0, 52);
    display.printf("RSSI: %d dBm | %s",
      WiFi.RSSI(),
      WiFi.RSSI() > -50 ? "Kuat" :
      WiFi.RSSI() > -70 ? "Sedang" : "Lemah");
  } else {
    display.printf("WiFi: DISCONNECTED");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }

  connectWiFi();
  if (wifiConnected) syncNTP();

  // Auto reconnect
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void loop() {
  // Cek WiFi
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnected = false;
    static unsigned long lastReconnect = 0;
    if (millis() - lastReconnect > 10000) {
      lastReconnect = millis();
      WiFi.reconnect();
    }
  } else {
    wifiConnected = true;
  }

  updateOLED();
  delay(500); // Update OLED 2× per detik
}
```

---

### Praktikum 3.2 — Mode AP: ESP32 Hotspot

**`src/main.cpp`:**
```cpp
/*
 * PROGRAM  : ESP32 Access Point (Hotspot) + OLED
 * DESKRIPSI: ESP32 membuat jaringan WiFi sendiri (hotspot).
 *            Device (HP/laptop) bisa terhubung ke hotspot ini.
 *            OLED menampilkan info AP dan jumlah client terhubung.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const char* AP_SSID = "ESP32-IoT-Lab";
const char* AP_PASS = "12345678";  // Minimum 8 karakter

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal!"); while(1) delay(1);
  }

  // Start Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS, 1, 0, 4); // channel 1, not hidden, max 4 clients

  Serial.println("Access Point aktif!");
  Serial.printf("SSID    : %s\n", AP_SSID);
  Serial.printf("Password: %s\n", AP_PASS);
  Serial.printf("IP      : %s\n", WiFi.softAPIP().toString().c_str());
}

void loop() {
  int clients = WiFi.softAPgetStationNum();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("ESP32 Access Point");
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);

  display.setCursor(0, 13);
  display.printf("SSID: %s", AP_SSID);

  display.setCursor(0, 23);
  display.printf("Pass: %s", AP_PASS);

  display.setCursor(0, 33);
  display.printf("IP  : %s", WiFi.softAPIP().toString().c_str());

  display.drawLine(0, 43, 128, 43, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 48);
  display.printf("Client:%d", clients);

  display.display();

  delay(2000);
}
```

---

## 4. Referensi

1. **Espressif.** *WiFi API.* [https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/wifi.html)
2. **Random Nerd Tutorials.** *ESP32 NTP Client.* [https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/](https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/)
3. **Random Nerd Tutorials.** *ESP32 Access Point.* [https://randomnerdtutorials.com/esp32-access-point-ap-web-server/](https://randomnerdtutorials.com/esp32-access-point-ap-web-server/)
