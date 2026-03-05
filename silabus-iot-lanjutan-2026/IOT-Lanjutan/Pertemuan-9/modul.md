# Modul Pertemuan 9 — IOT Lanjutan
# Home Assistant Advanced & ESPHome Lanjutan

---

## 1. Maksud dan Tujuan Materi

### Maksud
Membangun di atas pengenalan ESPHome/HA di IOT Dasar P12-13, pertemuan ini mendalami fitur lanjutan: **custom components ESPHome** (C++ lambda), **ESPHome OTA**, **ESPHome sebagai MQTT bridge**, integrasi sensor IoT Lanjutan (ESP-NOW nodes) ke HA via ESPHome gateway, dan **HA Add-ons** (Mosquitto, File Editor).

### Tujuan Pembelajaran
1. **Menulis** custom sensor ESPHome menggunakan C++ lambda.
2. **Mengonfigurasi** ESPHome dengan MQTT (dual mode: native API + MQTT).
3. **Mengelola** OTA update dan fallback hotspot ESPHome.
4. **Menginstal** dan mengonfigurasi HA Add-ons (Mosquitto Broker, File Editor).

---

## 2. Praktikum

### 2.1 — ESPHome Custom Sensor (Lambda C++)

```yaml
# custom-sensor.yaml
esphome:
  name: custom-sensor

esp32:
  board: esp32dev

wifi:
  ssid: "NamaWiFi"
  password: "Password"
  ap:
    ssid: "custom-fallback"

captive_portal:
api:
ota:
  - platform: esphome
logger:

i2c:
  sda: GPIO21
  scl: GPIO22

# Sensor custom: hitung jumlah boot dari NVS
sensor:
  - platform: template
    name: "Boot Counter"
    id: boot_counter
    unit_of_measurement: "kali"
    accuracy_decimals: 0
    lambda: |-
      static int count = -1;
      if (count < 0) {
        // Baca dari NVS pertama kali
        auto prefs = global_preferences->make_preference<int>(0x12345678);
        prefs.load(&count);
        count++;
        prefs.save(&count);
      }
      return count;
    update_interval: 60s

  # Sensor: uptime dalam format manusia
  - platform: uptime
    name: "Uptime"
    id: uptime_sensor

  # Sensor: kekuatan WiFi
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 30s

  # DHT22
  - platform: dht
    pin: GPIO14
    model: DHT22
    temperature:
      name: "Suhu Custom"
      id: temp
      filters:
        - offset: -0.5  # Kalibrasi
    humidity:
      name: "Kelembaban Custom"
      id: hum
    update_interval: 10s

# Binary sensor: button fisik
binary_sensor:
  - platform: gpio
    pin:
      number: GPIO15
      mode: INPUT_PULLUP
      inverted: true
    name: "Physical Button"
    on_multi_click:
      - timing:
          - ON for at most 0.5s
          - OFF for at most 0.5s
          - ON for at most 0.5s
        then:
          - logger.log: "Double click!"
          - switch.toggle: relay

switch:
  - platform: gpio
    pin: GPIO2
    name: "LED Custom"
    id: led

  - platform: gpio
    pin:
      number: GPIO4
      inverted: true
    name: "Relay Custom"
    id: relay

# Display OLED
display:
  - platform: ssd1306_i2c
    model: "SSD1306 128x64"
    address: 0x3C
    lambda: |-
      it.printf(0, 0, id(font1), "HA Advanced");
      it.line(0, 12, 128, 12);
      if (id(temp).has_state())
        it.printf(0, 16, id(font1), "T: %.1f C", id(temp).state);
      if (id(hum).has_state())
        it.printf(0, 28, id(font1), "H: %.1f %%", id(hum).state);
      it.printf(0, 40, id(font1), "LED: %s", id(led).state ? "ON" : "OFF");
      it.printf(0, 52, id(font1), "Relay: %s", id(relay).state ? "ON" : "OFF");

font:
  - file: "gfonts://Roboto"
    id: font1
    size: 10
```

### 2.2 — ESPHome + MQTT Bridge Mode

```yaml
# Tambahkan ke config ESPHome untuk dual mode (API + MQTT)
mqtt:
  broker: "192.168.1.100"  # IP Mosquitto broker
  port: 1883
  username: "mqtt_user"
  password: "mqtt_pass"
  discovery: true  # Auto-discovery di HA via MQTT
  topic_prefix: "esphome/custom-sensor"
```

### 2.3 — HA Add-ons Setup

**Install Mosquitto Add-on di HA:**
1. Settings → Add-ons → Add-on Store → "Mosquitto broker" → Install
2. Konfigurasi: Settings → Add-ons → Mosquitto → Configuration
3. Buat user MQTT: Settings → People → Users → Add user "mqtt"
4. Start Mosquitto

**Install File Editor:**
1. Add-on Store → "File Editor" → Install → Start
2. Akses konfigurasi YAML HA via web browser

---

## 3. Referensi

1. **ESPHome.** *Custom Sensor Component.* [https://esphome.io/components/sensor/custom.html](https://esphome.io/components/sensor/custom.html)
2. **ESPHome.** *MQTT Client.* [https://esphome.io/components/mqtt.html](https://esphome.io/components/mqtt.html)
3. **Home Assistant.** *Mosquitto Add-on.* [https://www.home-assistant.io/integrations/mqtt/](https://www.home-assistant.io/integrations/mqtt/)
