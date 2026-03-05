# Modul Pertemuan 13 — IOT Dasar
# Home Assistant & ESPHome — Smart Home Platform

---

## 1. Maksud dan Tujuan Materi

### Maksud
**Home Assistant (HA)** adalah platform smart home open-source yang menjadi standar industri untuk automasi rumah. Dikombinasikan dengan **ESPHome**, mahasiswa dapat membangun sistem IoT yang terintegrasi penuh — dari sensor ESP32 hingga dashboard dan automasi — **tanpa menulis kode C++**. Pertemuan ini membahas instalasi HA, konfigurasi ESPHome, dan integrasi keduanya.

### Tujuan Pembelajaran
1. **Menginstal** Home Assistant pada Docker / VM / Raspberry Pi.
2. **Mengonfigurasi** ESPHome YAML untuk sensor DHT22 dan aktuator LED/relay.
3. **Mengintegrasikan** ESPHome device ke Home Assistant (auto-discovery).
4. **Membangun** dashboard Lovelace di Home Assistant.
5. **Membuat** automasi dasar: jika sensor > threshold → aksi aktuator.

---

## 2. Teori Materi

### 2.1 Apa itu Home Assistant?

Home Assistant adalah platform open-source untuk otomasi rumah yang berjalan di server lokal (bukan cloud). Mendukung 2000+ integrasi (Xiaomi, Tuya, Google, Amazon, Zigbee, Z-Wave, MQTT, dan **ESPHome**).

**Keunggulan:**
- **Privasi penuh** — data tetap di jaringan lokal
- **Tanpa langganan** — gratis, open-source
- **Automasi powerful** — trigger + condition + action
- **Dashboard cantik** — Lovelace cards yang kustomisasi

### 2.2 Arsitektur: ESPHome ↔ Home Assistant

```
  ┌───────────────────────────────────────────────────────┐
  │  Home Assistant (Server — RPi / Docker / VM)          │
  │  ┌─────────────┐  ┌─────────────┐  ┌──────────────┐  │
  │  │  Dashboard   │  │  Automasi   │  │  History /   │  │
  │  │  (Lovelace)  │  │  Engine     │  │  Logbook     │  │
  │  └──────┬───────┘  └──────┬──────┘  └──────────────┘  │
  │         │                 │                            │
  │  ┌──────▼─────────────────▼───┐                        │
  │  │  ESPHome Integration       │ ← Native API           │
  │  │  (auto-discovery)          │    (bukan MQTT!)        │
  │  └──────────────┬─────────────┘                        │
  └─────────────────┼──────────────────────────────────────┘
                    │  WiFi (LAN)
  ┌─────────────────▼──────────────────────────────────────┐
  │  ESP32 + ESPHome Firmware                              │
  │  ┌───────────┐  ┌───────────┐  ┌───────────────────┐  │
  │  │  DHT22    │  │  LED/Relay│  │  OLED (opsional)  │  │
  │  │  (sensor) │  │  (switch) │  │  (display)        │  │
  │  └───────────┘  └───────────┘  └───────────────────┘  │
  └────────────────────────────────────────────────────────┘
```

### 2.3 Instalasi Home Assistant

**Opsi 1: Docker (Recommended untuk belajar)**
```bash
docker run -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=Asia/Jakarta \
  -v /path/to/config:/config \
  -p 8123:8123 \
  ghcr.io/home-assistant/home-assistant:stable
```

Buka browser: `http://localhost:8123`

**Opsi 2: Home Assistant OS on Raspberry Pi**
1. Download image dari [home-assistant.io/installation](https://www.home-assistant.io/installation/raspberrypi)
2. Flash ke SD Card dengan Balena Etcher
3. Boot Raspberry Pi → buka `http://homeassistant.local:8123`

**Opsi 3: VM (VirtualBox/VMware)**
1. Download `.ova` atau `.vmdk` dari website HA
2. Import ke VirtualBox → Network: Bridged Adapter
3. Boot → buka `http://IP_VM:8123`

### 2.4 ESPHome Add-on / Dashboard

**Install ESPHome di Home Assistant:**
1. Buka HA → **Settings → Add-ons → Add-on Store**
2. Cari **"ESPHome"** → Install → Start
3. Buka ESPHome Dashboard di sidebar HA

**Atau gunakan ESPHome CLI (standalone):**
```bash
pip install esphome
esphome dashboard ./configs/
# Buka http://localhost:6052
```

---

## 3. Panduan Praktikum

### Praktikum 3.1 — Konfigurasi ESPHome: Sensor + Aktuator

Buat file `smart-room.yaml`:

```yaml
# =============================================
#  ESPHome — Smart Room Controller
#  Sensor: DHT22 (suhu, kelembaban)
#  Aktuator: LED (GPIO 2), Relay (GPIO 4)
#  Display: OLED SSD1306 (opsional)
# =============================================

esphome:
  name: smart-room
  friendly_name: "Smart Room"

esp32:
  board: esp32dev
  framework:
    type: arduino

# --- Logging ---
logger:
  level: DEBUG

# --- WiFi ---
wifi:
  ssid: "NamaWiFi"
  password: "PasswordAnda"

  # Fallback hotspot jika WiFi gagal
  ap:
    ssid: "smart-room-fallback"
    password: "fallback123"

captive_portal:

# --- Home Assistant API ---
api:
  encryption:
    key: "masukkan-key-anda-disini"
  # Key di-generate otomatis oleh ESPHome Dashboard

# --- OTA Update ---
ota:
  - platform: esphome
    password: "ota-password-anda"

# ===================
#  SENSOR: DHT22
# ===================
sensor:
  - platform: dht
    pin: GPIO14
    model: DHT22
    temperature:
      name: "Suhu Ruangan"
      id: room_temp
      unit_of_measurement: "°C"
      accuracy_decimals: 1
      filters:
        - sliding_window_moving_average:
            window_size: 5
            send_every: 1
    humidity:
      name: "Kelembaban Ruangan"
      id: room_hum
      unit_of_measurement: "%"
      accuracy_decimals: 1
    update_interval: 5s

  # Sensor internal ESP32
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 30s

  - platform: uptime
    name: "Uptime"

# ===================
#  AKTUATOR: LED & Relay
# ===================
switch:
  - platform: gpio
    pin: GPIO2
    name: "LED Ruangan"
    id: led_room
    icon: "mdi:led-on"

  - platform: gpio
    pin: GPIO4
    name: "Relay Kipas"
    id: relay_fan
    icon: "mdi:fan"
    inverted: true  # Relay active-low

  # Tombol restart via HA
  - platform: restart
    name: "Restart ESP32"

# ===================
#  INPUT: Push Button
# ===================
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO15
      mode: INPUT_PULLUP
      inverted: true
    name: "Tombol Fisik"
    on_press:
      then:
        - switch.toggle: led_room

  - platform: status
    name: "Status Koneksi"

# ===================
#  TEXT SENSOR
# ===================
text_sensor:
  - platform: wifi_info
    ip_address:
      name: "IP Address"
    ssid:
      name: "WiFi SSID"

# ===================
#  DISPLAY: OLED SSD1306 (Opsional)
# ===================
i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

display:
  - platform: ssd1306_i2c
    model: "SSD1306 128x64"
    address: 0x3C
    update_interval: 1s
    lambda: |-
      it.printf(0, 0, id(font1), "Smart Room");
      it.line(0, 12, 128, 12);
      if (id(room_temp).has_state()) {
        it.printf(0, 16, id(font1), "Suhu : %.1f C", id(room_temp).state);
        it.printf(0, 28, id(font1), "Hum  : %.1f %%", id(room_hum).state);
      }
      it.printf(0, 42, id(font1), "LED  : %s", id(led_room).state ? "ON" : "OFF");
      it.printf(0, 54, id(font1), "Fan  : %s", id(relay_fan).state ? "ON" : "OFF");

font:
  - file: "gfonts://Roboto"
    id: font1
    size: 10
```

**Compile & Upload:**
```bash
# Pertama kali (via USB)
esphome run smart-room.yaml

# Setelah itu (via OTA, tanpa kabel!)
esphome run smart-room.yaml --device smart-room.local
```

---

### Praktikum 3.2 — Integrasi dengan Home Assistant

1. **ESPHome Auto-Discovery:**
   - Setelah ESP32 flash dan terhubung WiFi, buka Home Assistant
   - **Settings → Devices & Services** → notifikasi "1 new device found"
   - Klik **Configure** → masukkan encryption key → **Submit**
   - Device "Smart Room" muncul dengan semua entity!

2. **Dashboard Lovelace:**
   - Buka HA → **Overview** → klik **"Edit Dashboard"** (pojok kanan atas)
   - Tambahkan card:
     - **Gauge Card:** Pilih "Suhu Ruangan" → range 0–50°C
     - **Gauge Card:** Pilih "Kelembaban" → range 0–100%
     - **Button Card:** Pilih "LED Ruangan" → toggle ON/OFF
     - **Button Card:** Pilih "Relay Kipas" → toggle ON/OFF
     - **History Graph:** Pilih suhu dan kelembaban → lihat grafik 24 jam

---

### Praktikum 3.3 — Automasi Dasar di Home Assistant

**Automasi 1: Kipas Otomatis saat Panas**

```yaml
# automation.yaml atau buat via UI:
# Settings → Automations → Create Automation

alias: "Kipas Otomatis - Panas"
description: "Nyalakan kipas jika suhu > 30°C"
trigger:
  - platform: numeric_state
    entity_id: sensor.suhu_ruangan
    above: 30
condition: []
action:
  - service: switch.turn_on
    target:
      entity_id: switch.relay_kipas
  - service: notify.persistent_notification
    data:
      message: "Suhu ruangan {{ states('sensor.suhu_ruangan') }}°C — Kipas dinyalakan!"
```

**Automasi 2: LED Menyala Saat Malam**
```yaml
alias: "LED Malam"
trigger:
  - platform: sun
    event: sunset
action:
  - service: switch.turn_on
    target:
      entity_id: switch.led_ruangan
```

---

## 4. Referensi

1. **Home Assistant.** *Getting Started.* [https://www.home-assistant.io/getting-started/](https://www.home-assistant.io/getting-started/)
2. **ESPHome.** *Getting Started.* [https://esphome.io/guides/getting_started_hassio.html](https://esphome.io/guides/getting_started_hassio.html)
3. **ESPHome.** *DHT Sensor.* [https://esphome.io/components/sensor/dht.html](https://esphome.io/components/sensor/dht.html)
4. **ESPHome.** *SSD1306 Display.* [https://esphome.io/components/display/ssd1306.html](https://esphome.io/components/display/ssd1306.html)
5. **Home Assistant.** *Automations.* [https://www.home-assistant.io/docs/automation/](https://www.home-assistant.io/docs/automation/)
